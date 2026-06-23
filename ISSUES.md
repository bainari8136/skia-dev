# Code Review Issues

## Fixed

### Scrollbar drags could remain active after mouse capture was lost

- **Affected components:** `skCodeEditor`, `skConsoleView`, `skScrollPanel`, and `skTableView`.
- **Symptom:** If Windows cancelled an interaction or another window took mouse capture before button-up was delivered, a scrollbar drag remained active. A later mouse move could unexpectedly resume the old drag.
- **Cause:** The window acquired capture on mouse-down, but capture cancellation was not represented in the UI event model and the scrollbar implementations only cleared their state on `MouseUp`.
- **Resolution:** Added `MouseCancel`, dispatch it for `WM_CANCELMODE` and `WM_CAPTURECHANGED`, and clear every new scrollbar drag state on either mouse-up or cancellation.

### Scroll panels reserved a dead scrollbar strip without overflow

- **Affected component:** `skScrollPanel`.
- **Symptom:** When the content fit within the panel, the rightmost 10 pixels were visible but did not accept child mouse input.
- **Cause:** Event hit testing always classified that area as the scrollbar strip even though painting only reserved and drew the strip when content overflowed.
- **Resolution:** The strip is now excluded from child input only while a scrollbar is actually needed.

### Table scrollbar drag geometry differed from painted geometry

- **Affected component:** `skTableView`.
- **Symptom:** With many rows, the thumb moved faster than the pointer and reached the scroll limit before reaching the end of the painted track.
- **Cause:** Dragging used a 20-pixel minimum thumb height while painting allowed a smaller proportional thumb, producing different track lengths.
- **Resolution:** Painting and dragging now use the same clamped 20-pixel-minimum thumb height calculation.
