## **UI_CONTROLS.md** 

## **skWidgets UI Controls Catalog** 

## **Purpose** 

This document defines all UI controls planned for implementation in skWidgets. 

Controls are organized into implementation phases. 

The goal is to prioritize controls that enable real desktop applications while maintaining a clean and scalable architecture. 

## **Control Classification** 

Controls are grouped into: 

1. Core Controls 

2. Input Controls 

3. Layout Controls 

4. Data Controls 

5. Navigation Controls 

6. Dialog Controls 

7. Advanced Controls 

8. Developer Controls 

## **Phase 1: Core Controls** 

These controls are required to bootstrap the framework. 

## **skWidget** 

Base class for all controls. 

Responsibilities: 

- Position 

- Size 

- Visibility 

1 

- Focus 

- Event handling 

- Painting 

## **skPanel** 

Generic container widget. 

Features: 

- Child widgets 

- Background drawing 

- Clipping 

## **skLabel** 

Displays text. 

Features: 

- Single-line text 

- Multi-line text 

- Alignment 

- Font selection 

## **skButton** 

Clickable command button. 

Features: 

- Hover state 

- Pressed state 

- Disabled state 

- Click events 

## **skImage** 

Displays images. 

2 

Features: 

- PNG 

- JPEG 

- SVG (future) 

- Scaling modes 

## **Phase 2: Input Controls** 

## **skTextBox** 

Single-line text input. 

Features: 

- Caret 

- Selection 

- Clipboard support 

## **skTextArea** 

Multi-line text input. 

Features: 

- Scrolling 

- Selection 

- Clipboard support 

## **skPasswordBox** 

Password entry control. 

Features: 

- Character masking 

- Reveal button 

3 

## **skCheckBox** 

Boolean selection control. 

Features: 

- Checked state 

- Unchecked state 

## **skRadioButton** 

Single-choice selection control. 

Features: 

- Group support 

## **skToggleButton** 

On/off button. 

Features: 

- Toggle state 

## **skComboBox** 

Dropdown selection. 

Features: 

- Expand/collapse 

- Item selection 

## **skSlider** 

Range selection control. 

4 

Features: 

- Horizontal 

- Vertical 

## **skSpinBox** 

Numeric input control. 

Features: 

- Increment/decrement buttons 

## **Phase 3: Layout Controls** 

## **skBoxSizer** 

Linear layout manager. 

Features: 

- Vertical layout 

- Horizontal layout 

## **skGridSizer** 

Grid-based layout manager. 

Features: 

- Rows 

- Columns 

## **skFlexSizer** 

Responsive layout manager. 

5 

Features: 

- Grow 

- Shrink 

- Alignment 

## **skStackPanel** 

Stacks child widgets. 

Features: 

• Overlay layouts 

## **skScrollView** 

Scrollable container. 

Features: 

- Horizontal scrolling 

- Vertical scrolling 

## **Phase 4: Navigation Controls** 

## **skTabView** 

Tabbed interface. 

Features: 

- Multiple pages 

- Closable tabs 

## **skSideBar** 

Navigation sidebar. 

6 

Features: 

- Icons 

- Expand/collapse 

## **skToolBar** 

Application toolbar. 

Features: 

- Icons 

- Actions 

## **skMenuBar** 

Application menu bar. 

Features: 

- Menus 

- Submenus 

## **skStatusBar** 

Application status bar. 

Features: 

- Status text 

- Progress indicators 

## **skBreadcrumb** 

Path navigation control. 

Features: 

- Hierarchical navigation 

7 

## **Phase 5: Data Controls** 

## **skListView** 

Displays lists of items. 

Features: 

- Selection 

- Sorting 

## **skTreeView** 

Hierarchical data display. 

Features: 

- Expand/collapse 

## **skTableView** 

Tabular data display. 

Features: 

- Rows 

- Columns 

- Sorting 

## **skPropertyGrid** 

Property editor. 

Features: 

- Name/value editing 

8 

## **skDataGrid** 

Advanced table component. 

Features: 

- Cell editing 

- Filtering 

## **Phase 6: Dialog Controls** 

## **skDialog** 

Modal dialog window. 

Features: 

- ShowModal() 

## **skMessageBox** 

Simple message dialog. 

Features: 

- Information 

- Warning 

- Error 

## **skFileDialog** 

File selection dialog. 

Features: 

- Open 

- Save 

9 

## **skColorDialog** 

Color picker dialog. 

## **skFontDialog** 

Font picker dialog. 

## **skDatePicker** 

Date selection dialog. 

## **Phase 7: Modern Controls** 

## **skCard** 

Material-style card. 

Features: 

- Elevation 

- Shadows 

## **skDrawer** 

Slide-out navigation panel. 

## **skNavigationRail** 

Desktop navigation rail. 

## **skToast** 

Temporary notification. 

10 

## **skPopover** 

Floating contextual popup. 

## **skTooltip** 

Context help popup. 

## **skProgressBar** 

Progress indicator. 

Features: 

- Determinate 

- Indeterminate 

## **skCircularProgress** 

Circular loading indicator. 

## **skChip** 

Small action/display component. 

## **skAvatar** 

User profile image. 

## **skBadge** 

Notification badge. 

11 

## **Phase 8: Developer Controls** 

## **skCodeEditor** 

Source code editor. 

Features: 

- Syntax highlighting • Line numbers 

## **skConsoleView** 

Terminal output control. 

Features: 

- Colored output 

## **skInspector** 

Widget inspection tool. 

Features: 

- Widget hierarchy 

- Layout debugging 

## **skDockPanel** 

Dockable windows. 

Features: 

- IDE-style layout 

## **skSplitView** 

Resizable panes. 

12 

Features: 

- Horizontal split 

- Vertical split 

## **Future Controls** 

## **skChartView** 

Charts and graphs. 

Features: 

- Line chart 

- Bar chart 

- Pie chart 

## **skCanvasView** 

Custom drawing surface. 

Features: 

- Free drawing 

- Custom rendering 

## **skVideoView** 

Video playback control. 

## **skWebView** 

Embedded web browser. 

## **skMarkdownView** 

Markdown renderer. 

13 

## **skPdfView** 

PDF viewer. 

## **Long-Term Goal** 

The complete skWidgets control set should be capable of building: 

- Desktop applications 

- IDEs 

- Media tools 

- Database clients 

- ERP systems 

- Engineering tools 

- Design tools 

- Educational software 

without depending on native platform widgets. 

14 

