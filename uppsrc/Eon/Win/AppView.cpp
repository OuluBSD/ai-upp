////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "EonWin.h"

using namespace concurrency;
using namespace std::placeholders;

namespace winrt_app = winrt::Windows::ApplicationModel;
namespace winrt_core = winrt::Windows::ApplicationModel::Core;
namespace winrt_ui_core = winrt::Windows::UI::Core;
namespace winrt_activation = winrt::Windows::ApplicationModel::Activation;
namespace winrt_foundation = winrt::Windows::Foundation;
namespace winrt_holo = winrt::Windows::Graphics::Holographic;

NAMESPACE_UPP

static UwpVrAppFactory& UwpVrAppFactoryRef()
{
	static UwpVrAppFactory factory = nullptr;
	return factory;
}

void SetUwpVrAppFactory(UwpVrAppFactory factory)
{
	UwpVrAppFactoryRef() = factory;
}

UwpVrAppFactory GetUwpVrAppFactory()
{
	return UwpVrAppFactoryRef();
}

// IFrameworkViewSource methods

winrt_core::IFrameworkView AppViewSource::CreateView()
{
    return holographicView;
}

// IFrameworkView methods

// The first method called when the IFrameworkView is being created.
// Use this method to subscribe for Windows shell events and to initialize your app.
void AppView::Initialize(winrt_core::CoreApplicationView const& applicationView)
{
    applicationView.Activated(std::bind(&AppView::OnViewActivated, this, _1, _2));

    // Register event handlers for app lifecycle.
    m_suspendingEventToken = winrt_core::CoreApplication::Suspending(std::bind(&AppView::OnSuspending, this, _1, _2));

    m_resumingEventToken = winrt_core::CoreApplication::Resuming(std::bind(&AppView::OnResuming, this, _1, _2));

    auto factory = GetUwpVrAppFactory();
    if(factory)
        m_main = factory();
    if(!m_main)
        m_main = std::make_unique<ShellConnectorApp>();
}

void AppView::OnKeyPressed(winrt_ui_core::CoreWindow const& sender, winrt_ui_core::KeyEventArgs const& args)
{
}

void AppView::OnPointerPressed(winrt_ui_core::CoreWindow const& sender, winrt_ui_core::PointerEventArgs const& args)
{
}

// Called when the CoreWindow object is created (or re-created).
void AppView::SetWindow(winrt_ui_core::CoreWindow const& window)
{
    // Register for keypress notifications.
    m_keyDownEventToken = window.KeyDown(std::bind(&AppView::OnKeyPressed, this, _1, _2));

    // Register for pointer pressed notifications.
    m_pointerPressedEventToken = window.PointerPressed(std::bind(&AppView::OnPointerPressed, this, _1, _2));

    // Register for notification that the app window is being closed.
    m_windowClosedEventToken = window.Closed(std::bind(&AppView::OnWindowClosed, this, _1, _2));

    // Register for notifications that the app window is losing focus.
    m_visibilityChangedEventToken = window.VisibilityChanged(std::bind(&AppView::OnVisibilityChanged, this, _1, _2));

    // Create a holographic space for the core window for the current view.
    // Presenting holographic frames that are created by this holographic space will put
    // the app into exclusive mode.
    m_holographicSpace = winrt_holo::HolographicSpace::CreateForCoreWindow(window);

    // The main class uses the holographic space for updates and rendering.
    m_main->SetHolographicSpace(m_holographicSpace);
}

// The Load method can be used to initialize scene resources or to load a
// previously saved app state.
void AppView::Load(winrt::hstring const& entryPoint)
{
}

// This method is called after the window becomes active. It oversees the
// update, draw, and present loop, and it also oversees window message processing.
void AppView::Run()
{
    while (!m_windowClosed)
    {
        if (m_windowVisible && (m_holographicSpace != nullptr))
        {
            winrt_ui_core::CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(winrt_ui_core::CoreProcessEventsOption::ProcessAllIfPresent);

            m_main->Update();
        }
        else
        {
            winrt_ui_core::CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(winrt_ui_core::CoreProcessEventsOption::ProcessOneAndAllPending);
        }
    }
}

// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground, for example if the Run method exits.
void AppView::Uninitialize()
{
    m_main.reset();

    winrt_core::CoreApplication::Suspending(m_suspendingEventToken);
    winrt_core::CoreApplication::Resuming(m_resumingEventToken);

    auto const& window = winrt_ui_core::CoreWindow::GetForCurrentThread();
    window.KeyDown(m_keyDownEventToken);
    window.PointerPressed(m_pointerPressedEventToken);
    window.Closed(m_windowClosedEventToken);
    window.VisibilityChanged(m_visibilityChangedEventToken);
}


// Application lifecycle event handlers

// Called when the app is prelaunched. Use this method to load resources ahead of time
// and enable faster launch times.
void AppView::OnLaunched(winrt_activation::LaunchActivatedEventArgs const& args)
{
    if (args.PrelaunchActivated())
    {
        //
        // TODO: Insert code to preload resources here.
        //
    }
}

// Called when the app view is activated. Activates the app's CoreWindow.
void AppView::OnViewActivated(winrt_core::CoreApplicationView const& sender, winrt_activation::IActivatedEventArgs const& args)
{
    // Run() won't start until the CoreWindow is activated.
    sender.CoreWindow().Activate();
}

void AppView::OnSuspending(winrt_foundation::IInspectable const& sender, winrt_app::SuspendingEventArgs const& args)
{
    // Save app state asynchronously after requesting a deferral. Holding a deferral
    // indicates that the application is busy performing suspending operations. Be
    // aware that a deferral may not be held indefinitely; after about five seconds,
    // the app will be forced to exit.
    winrt_app::SuspendingDeferral deferral = args.SuspendingOperation().GetDeferral();

    create_task([this, deferral]
    {
        if (m_main)
        {
            m_main->SaveAppState();
        }

        deferral.Complete();
    });
}

void AppView::OnResuming(winrt_foundation::IInspectable const& sender, winrt_foundation::IInspectable const& args)
{
    // Restore any data or state that was unloaded on suspend. By default, data
    // and state are persisted when resuming from suspend. Note that this event
    // does not occur if the app was previously terminated.

    if (m_main)
    {
        m_main->LoadAppState();
    }
}


// Window event handlers

void AppView::OnVisibilityChanged(winrt_ui_core::CoreWindow const& sender, winrt_ui_core::VisibilityChangedEventArgs const& args)
{
    m_windowVisible = args.Visible();
}

void AppView::OnWindowClosed(winrt_ui_core::CoreWindow const& sender, winrt_ui_core::CoreWindowEventArgs const& args)
{
	m_windowClosed = true;
}

void ShellConnectorApp::SetHolographicSpace(
    winrt_holo::HolographicSpace const& holographicSpace)
{
}

void ShellConnectorApp::Update()
{
}

void ShellConnectorApp::SaveAppState()
{
}

void ShellConnectorApp::LoadAppState()
{
}

END_UPP_NAMESPACE