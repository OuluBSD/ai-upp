#include "LLDB.h"


std::optional<int> FileViewer::render(void)
{
    const std::unordered_set<int>* const bps =
        (m_breakpoints.has_value() && m_breakpoints != m_breakpoint_cache.end())
            ? &(*m_breakpoints)->second
            : nullptr;

    ImGuiContext& g = *GImGui;
    auto& style = g.Style;
    ImGuiWindow* window = g.CurrentWindow;

    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, style.Colors[ImGuiCol_TitleBg]);
    Defer(ImGui::PopStyleColor());
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, style.Colors[ImGuiCol_TitleBg]);
    Defer(ImGui::PopStyleColor());

    std::optional<int> clicked_line = {};

    String line_buffer;
    for (size_t i = 0; i < m_lines.size(); i++) {
        const size_t line_number = i + 1;

        line_buffer = Format("   %d  %s\n", (int)line_number, (String)m_lines[i]);

        bool selected = false;
        if (m_highlighted_line.has_value() &&
            line_number == static_cast<size_t>(*m_highlighted_line)) {
            selected = true;
            if (m_highlight_line_needs_focus) {
                //ImGui::SetScrollHere();
                m_highlight_line_needs_focus = false;
            }
        }

        ImGui::Selectable(~line_buffer, selected);
        if (ImGui::IsItemClicked()) {
            clicked_line = line_number;
        }

        if (bps != nullptr && bps->find(line_number) != bps->end()) {
            ImVec2 pad = style.FramePadding;
            ImVec2 pos = window->DC.CursorPos;
            ImVec2 txt = ImGui::CalcTextSize("X");
            pos.x += 1.3f * txt.x;
            pos.y -= (txt.y + 2.f * pad.y) / 2.f;
            window->DrawList->AddCircleFilled(pos, txt.y / 3.f, IM_COL32(255, 0, 0, 255));
        }

        line_buffer.Clear();
    }

    return clicked_line;
}

void FileViewer::synchronize_breakpoint_cache(lldb::SBTarget target)
{
    for (auto& [_, bps] : m_breakpoint_cache) bps.clear();

    for (int i = 0; i < target.GetNumBreakpoints(); i++) {
        lldb::SBBreakpoint bp = target.GetBreakpointAtIndex(i);
        lldb::SBBreakpointLocation location = bp.GetLocationAtIndex(0);

        if (!location.IsValid()) {
            LOG("error: Invalid breakpoint location encountered by LLDB.");
        }

        lldb::SBAddress address = location.GetAddress();

        if (!address.IsValid()) {
            LOG("error: Invalid lldb::SBAddress for breakpoint encountered.");
        }

        lldb::SBLineEntry line_entry = address.GetLineEntry();

        if (!line_entry.IsValid()) {
            LOG("error: Invalid lldb::SBLineEntry for breakpoint encountered.");
            continue;
        }

        // auto file_spec = line_entry.GetFileSpec();

        const String bp_filepath = Format("%s/%s",
			(String)line_entry.GetFileSpec().GetDirectory(),
			(String)line_entry.GetFileSpec().GetFilename());

        const auto maybe_handle = FileHandle::create(~bp_filepath);
        if (!maybe_handle) {
            LOG("error: Invalid filepath found for breakpoint: " << bp_filepath);
            continue;
        }
        const FileHandle handle = *maybe_handle;

        if (auto it = m_breakpoint_cache.find(handle); it == m_breakpoint_cache.end()) {
            m_breakpoint_cache.emplace(handle,
                                       std::unordered_set<int>({(int)line_entry.GetLine()}));
        }
        else {
            it->second.insert((int)line_entry.GetLine());
        }
    }
}

void FileViewer::show(FileHandle handle)
{
    m_lines = handle.contents();

    if (const auto it = m_breakpoint_cache.find(handle); it != m_breakpoint_cache.end()) {
        m_breakpoints = it;
    }
    else {
        m_breakpoints = {};
    }
}
