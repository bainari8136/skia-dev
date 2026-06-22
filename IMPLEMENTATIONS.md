# IMPLEMENTATIONS.md

Tracks implementation status of every control listed in `UI Controls Catalog.md`.

**Legend:** `[x]` = implemented · `[ ]` = not yet implemented · `[~]` = partially implemented or implemented under a different name

---

## Phase 1: Core Controls

| Control | Status | Notes |
|---------|--------|-------|
| skWidget | [x] | `ui/skWidget.h` — base class, position, size, visibility, focus, events, paint |
| skPanel | [x] | `ui/skPanel.h/.cpp` — generic container with background and clipping |
| skLabel | [x] | `ui/skLabel.h/.cpp` — single/multi-line text, font size |
| skButton | [x] | `ui/skButton.h/.cpp` — hover/pressed states, smooth animation, click callback |
| skImage | [x] | `ui/skImage.h/.cpp` — PNG/JPEG via Skia encoded data; Stretch/Fit/Fill/Center scale modes; placeholder when no file loaded |

---

## Phase 2: Input Controls

| Control | Status | Notes |
|---------|--------|-------|
| skTextBox | [x] | Implemented as `skTextInput` (`ui/skTextInput.h/.cpp`) — caret, cursor blink, VK navigation |
| skTextArea | [x] | `ui/skTextArea.h/.cpp` — multi-line, scrolling, click-to-position cursor |
| skPasswordBox | [x] | `ui/skPasswordBox.h/.cpp` — subclass of skTextInput with `setMasked(true)`; displays `*` characters |
| skCheckBox | [x] | `ui/skCheckBox.h/.cpp` |
| skRadioButton | [x] | `ui/skRadioButton.h/.cpp` — group support via `skRadioGroup` |
| skToggleButton | [x] | Implemented as `skToggle` (`ui/skToggle.h/.cpp`) — animated thumb, on/off state |
| skComboBox | [x] | Implemented as `skDropdown` (`ui/skDropdown.h/.cpp`) — overlay-based expand/collapse |
| skSlider | [x] | `ui/skSlider.h/.cpp` — horizontal, drag, onChange callback |
| skSpinBox | [x] | Implemented as `skNumberInput` (`ui/skNumberInput.h/.cpp`) — increment/decrement buttons, VK_UP/DOWN |

---

## Phase 3: Layout Controls

| Control | Status | Notes |
|---------|--------|-------|
| skBoxSizer | [x] | Implemented as `skSizer` (`ui/skSizer.h/.cpp`) — Row and Column layout, gap support |
| skGridSizer | [x] | `ui/skGridSizer.h/.cpp` — N-column grid, auto row heights, flows left-to-right top-to-bottom |
| skFlexSizer | [x] | `ui/skFlexSizer.h/.cpp` — Row/Column layout with fixed-size and proportional flex items |
| skStackPanel | [x] | `ui/skStackPanel.h/.cpp` — vertical/horizontal child stacking, auto-sizes height, optional bg/border/radius |
| skScrollView | [x] | Implemented as `skScrollPanel` (`ui/skScrollPanel.h/.cpp`) — vertical scroll, children in local coords, mouse wheel |

---

## Phase 4: Navigation Controls

| Control | Status | Notes |
|---------|--------|-------|
| skTabView | [x] | Implemented as `skTabBar` (`ui/skTabBar.h/.cpp`) — tab switching, accent underline, onChange callback |
| skSideBar | [x] | `ui/skSideBar.h/.cpp` — vertical nav list, active indicator bar, hover highlight, onChange callback |
| skNavigationRail | [x] | `ui/skNavigationRail.h/.cpp` — vertical rail, icon glyph + label, pill highlight for active item, onChange callback |
| skToolBar | [x] | `ui/skToolBar.h/.cpp` — horizontal button strip with separators, hover highlight, click callbacks |
| skMenuBar | [x] | `ui/skMenuBar.h/.cpp` — title buttons, hover/active tint, opens skMenu overlays; keyboard Escape closes open menu |
| skStatusBar | [x] | `ui/skStatusBar.h/.cpp` — left + right text sections, accent stripe, resize-aware |
| skBreadcrumb | [x] | `ui/skBreadcrumb.h/.cpp` — "Home › Section › Page"; non-last crumbs are clickable underlined links |

---

## Phase 5: Data Controls

| Control | Status | Notes |
|---------|--------|-------|
| skListView | [x] | Implemented as `skListBox` (`ui/skListBox.h/.cpp`) — selection, VK_UP/DOWN, mouse wheel, scrollbar |
| skTreeView | [x] | `ui/skTreeView.h/.cpp` — hierarchical nodes with unique_ptr ownership; expand/collapse via click or Left/Right keys; VK_UP/DOWN navigation; scrollbar; `addRoot` / `addChild` builder API |
| skTableView | [x] | `ui/skTableView.h/.cpp` — headers, row selection, scrollbar, VK_UP/DOWN keyboard nav, focus border |
| skPropertyGrid | [x] | `ui/skPropertyGrid.h/.cpp` — two-column key-value editor, click to select, click again (or Enter) to edit, char input |
| skDataGrid | [x] | `ui/skDataGrid.h/.cpp` — inline-editable cells, click-to-select, click-again to edit, Tab/Enter navigate, Escape cancel |

---

## Phase 6: Dialog Controls

| Control | Status | Notes |
|---------|--------|-------|
| skDialog | [x] | Implemented as `skModal` (`ui/skModal.h/.cpp`) — backdrop, centered dialog, Confirm/Cancel, Escape key |
| skMessageBox | [x] | `ui/skMessageBox.h/.cpp` — Info/Warning/Error types, icon circle, colored top bar, OK button, Escape key |
| skFileDialog | [x] | `ui/skFileDialog.h/.cpp` — real Win32 directory listing, navigate dirs, select files, up button, scrollbar |
| skColorDialog | [x] | `ui/skColorDialog.h/.cpp` — HSV picker, SV square + hue bar + swatch, OK/Cancel, Escape, outside-click close |
| skFontDialog | [x] | `ui/skFontDialog.h/.cpp` — Win32 EnumFontFamiliesEx font list, preview area, bold/italic checkboxes, size +/-, OK/Cancel |
| skDatePicker | [x] | `ui/skDatePicker.h/.cpp` — calendar overlay, month/year nav, day grid, today/selected highlights, OK/Cancel |

---

## Phase 7: Modern Controls

| Control | Status | Notes |
|---------|--------|-------|
| skCard | [x] | `ui/skCard.h/.cpp` — rounded rect with multi-layer soft drop shadow; `setElevation()` controls shadow depth |
| skDrawer | [x] | `ui/skDrawer.h/.cpp` — slide-in panel from left, animated offset, accent title bar, item list, Escape/outside-click close |
| skToast | [x] | `ui/skToast.h/.cpp` — slide-in/hold/slide-out animation, managed by skWindow |
| skPopover | [x] | `ui/skPopover.h/.cpp` — floating panel above anchor, downward arrow, title + text lines, Escape/outside-click close |
| skTooltip | [x] | `ui/skTooltip.h/.cpp` — standalone overlay, showAt(x,y,text,ticks), auto-dismiss via onTick(), passive (never consumes events) |
| skProgressBar | [x] | `ui/skProgressBar.h/.cpp` — determinate; indeterminate not yet supported |
| skCircularProgress | [x] | Implemented as `skSpinner` (`ui/skSpinner.h/.cpp`) — rotating arc, start/stop/runFor |
| skChip | [x] | `ui/skChip.h/.cpp` — auto-sized pill label, accent-tinted bg, optional click callback, hover state |
| skAvatar | [x] | `ui/skAvatar.h/.cpp` — circular avatar, auto-picks palette color from initials, white initials text |
| skBadge | [x] | `ui/skBadge.h/.cpp` — pill shape, auto-sizes to text, custom color |

---

## Phase 8: Developer Controls

| Control | Status | Notes |
|---------|--------|-------|
| skCodeEditor | [x] | `ui/skCodeEditor.h/.cpp` — read-only viewer, line numbers, C++ syntax highlighting (keywords/types/strings/comments/preprocessor), loadFile(), mouse-wheel scroll |
| skConsoleView | [x] | `ui/skConsoleView.h/.cpp` — scrollable log output, dark bg, per-line colors, info/warn/error/success helpers, mouse wheel scroll |
| skInspector | [x] | `ui/skInspector.h/.cpp` — live theme token inspector; swatch + name + ARGB hex per token; scroll, hover highlight |
| skDockPanel | [x] | `ui/skDockPanel.h/.cpp` — Left/Right/Top/Bottom/Fill dock modes; children consume from remaining rect in insertion order |
| skSplitView | [x] | `ui/skSplitView.h/.cpp` — horizontal split, draggable divider, min-width clamp, delegates events to children |

---

## Future Controls

| Control | Status | Notes |
|---------|--------|-------|
| skChartView | [x] | `ui/skChartView.h/.cpp` — bar chart, auto Y-scaling, grid lines, value labels, hover highlight, title |
| skCanvasView | [x] | `ui/skCanvasView.h/.cpp` — custom paint callback surface, mouse down/move/up callbacks, clip + translate to local coords |
| skVideoView | [x] | `ui/skVideoView.h/.cpp` — play/pause/stop controls, seek bar, time display, simulated playback via onTick(); placeholder frame area |
| skWebView | [x] | `ui/skWebView.h/.cpp` — URL bar with back/forward/go, editable URL input, navigate() API, history stack; placeholder content area |
| skMarkdownView | [x] | `ui/skMarkdownView.h/.cpp` — H1/H2/H3, paragraphs (word-wrap), bullet lists, code blocks, horizontal rules; mouse-wheel scroll |
| skPdfView | [x] | `ui/skPdfView.h/.cpp` — page navigation (prev/next/wheel), zoom, paper simulation with placeholder lines; loadFile/goToPage API |

---

## Extra Controls (implemented, not in catalog)

These were added during development and have no catalog entry.

| Control | File | Description |
|---------|------|-------------|
| skLink | `ui/skLink.h/.cpp` | Clickable underlined text with accent color and onClick callback |
| skSeparator | `ui/skSeparator.h/.cpp` | Horizontal divider line |
| skExpander | `ui/skExpander.h/.cpp` | Animated collapsible section with rotating chevron |
| skTheme | `ui/skTheme.h/.cpp` | Light/Dark theme tokens, skGetTheme/skSetTheme, skLerpColor |
| skMenu | `ui/skMenu.h/.cpp` | Dropdown overlay with items, separators, hover highlight; add via skWindow::addOverlay |

---

## Summary

| Phase | Total | Implemented | Partial | Not Implemented |
|-------|-------|-------------|---------|-----------------|
| Phase 1: Core | 5 | 5 | 0 | 0 |
| Phase 2: Input | 9 | 9 | 0 | 0 |
| Phase 3: Layout | 5 | 5 | 0 | 0 |
| Phase 4: Navigation | 7 | 7 | 0 | 0 |
| Phase 5: Data | 5 | 5 | 0 | 0 |
| Phase 6: Dialog | 6 | 6 | 0 | 0 |
| Phase 7: Modern | 11 | 10 | 0 | 1 |
| Phase 8: Developer | 5 | 5 | 0 | 0 |
| Future | 6 | 6 | 0 | 0 |
| **Total** | **59** | **58** | **0** | **1** |
