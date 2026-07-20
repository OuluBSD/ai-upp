# Live Video Streaming and Player

Date Span: 2026-07-17 to 2026-07-18

✦ *Spearhead (Seppo)* pioneered the transition to live streaming video recognition and visualization. The work began with driving the real game engine with parsed real actions [10], which was quickly expanded by adding crop targeting options [9] and Otsu-based binarization preprocessing with edge-based polarity detection for OCR [8]. 

To achieve production-grade robustness, crop boundaries were adjusted after diagnostic gallery reviews [7], blank-crop detection was introduced [6], and critical collision bugs in temporary files were resolved [5]. With these enhancements, the validated **Tesseract OCR** pipeline was successfully wired into production [4].

Next, the focus shifted to live streaming. A real, high-performance looping video decoder using **libavcodec** was integrated into **VideoServer** [3]. To consume this live feed, **VsmVideoServerFrameSource** was developed as a continuous TCP frame-polling source [2]. This enabled the implementation of **VideoLiveRecognitionLoop**, a continuous per-frame recognition loop running alongside a real-time region classifier [1]. 

Finally, to allow the *Director* to visually watch the live feed consumed by the recognition loop, a standalone, mpv-style **VideoPlayer** window was created [11]. It connects to the running video server in parallel with other clients, utilizing aspect-ratio-preserving GPU drawing and a minimalist HUD overlay displaying live FPS and latency metrics. It was also updated to support direct local video file launching by spawning a background `VideoServer` subprocess to decode the video in-process via `libavcodec`.

## References
- [1] 16b83d427 — Continuous per-frame recognition loop + real-time region classifier (Task 0280) (Seppo Pakonen, 2026-07-18)
- [2] 4554dbc6d — Add VsmVideoServerFrameSource: continuous live VsmFrameSource (Task 0279) (Seppo Pakonen, 2026-07-18)
- [3] 8a74517bd — VideoServer: real libavcodec looping video decoder (Task 0278) (Seppo Pakonen, 2026-07-18)
- [4] 1b90d62cb — Wire the validated tesseract OCR pipeline into production (Task 0277) (Seppo Pakonen, 2026-07-18)
- [5] a3bf3fb35 — Fix blank-crop detection to OCR probe / Fix critical intermediate-file collision bug in OCR probe (Task 0275) (Seppo Pakonen, 2026-07-18)
- [6] 09208fece — Add blank-crop detection to OCR probe (Task 0274) (Seppo Pakonen, 2026-07-17)
- [7] a7e4e885c — Fix OCR crop boundaries per user's detailed diagnostic gallery review (Task 0273) (Seppo Pakonen, 2026-07-17)
- [8] 9e1f7f6c3 — Port Otsu preprocessing + new edge-based polarity detector for OCR (Task 0272) (Seppo Pakonen, 2026-07-17)
- [9] 80907401c — Add --crop-list to VideoSemanticOcrProbe for explicit crop targeting (Task 0271) (Seppo Pakonen, 2026-07-17)
- [10] 6d12a93dd — Drive the real game engine with parsed real actions (Task 0270) (Seppo Pakonen, 2026-07-17)
- [11] Local — Standalone live VideoServer viewer window (Task 0284) (Seppo Pakonen, 2026-07-18)
