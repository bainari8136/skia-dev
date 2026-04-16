#include  <include/core/SkImageInfo.h>
#include  <include/core/SkCanvas.h>
#include  <include/core/SkRRect.h>
#include  <include/core/SkPaint.h>
#include  <include/core/SkSurface.h>
#include  <include/core/SkStream.h>

#include <Windows.h>
#include <tchar.h>

static sk_sp<SkSurface>g_surface;
static int g_width=800;
static int g_height=600;

void drawSCene( SkCanvas *canvas){

    canvas->clear(SK_ColorWHITE);

    SkPaint paint;

    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);

    //canvas->drawCircle(400,300,100,paint);

    SkRRect btn;
    btn.setRectXY(SkRect::MakeXYWH(170,185,120,30),6,6);

    canvas->drawRRect(btn,paint);
   


}


void blitToWindow(HWND hwnd){

    if(!g_surface)return;

    SkPixmap pixmap;

    if(!g_surface->peekPixels(&pixmap))return;

    BITMAPINFO bmi={};

    bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth=g_width;
    bmi.bmiHeader.biHeight=-g_height;
    bmi.bmiHeader.biPlanes=1;
    bmi.bmiHeader.biBitCount=32;
    bmi.bmiHeader.biCompression=BI_RGB;

    PAINTSTRUCT ps;

    HDC hdc=BeginPaint(hwnd,&ps);
    HDC memdc=CreateCompatibleDC(hdc);

    void *bits=nullptr;

    HBITMAP hbmp=CreateDIBSection(hdc,&bmi,DIB_RGB_COLORS,&bits,nullptr,0);
    HBITMAP hold=(HBITMAP)SelectObject(memdc,hbmp);

    memcpy(bits,pixmap.addr(),(size_t)g_width*g_height*4);
    BitBlt(hdc,0,0,g_width,g_height,memdc,0,0,SRCCOPY);

    SelectObject(memdc,hold);
    DeleteObject(hbmp);
    DeleteDC(memdc);
    EndPaint(hwnd,&ps);

}

void recreateSurface(){

     g_surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(g_width,g_height));
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){

    switch(msg)
    {
      case WM_CREATE:
       recreateSurface();
       return 0;

     case WM_SIZE:
       if(LOWORD(lp)>0&&HIWORD(lp)>0){

        g_width=LOWORD(lp);
        g_height=HIWORD(lp);
        recreateSurface();
        InvalidateRect(hwnd,nullptr,FALSE);

       }

       return 0;

       case WM_PAINT:
        if(g_surface){
            drawSCene(g_surface->getCanvas());
            blitToWindow(hwnd);
        }
        return 0;

        case WM_ERASEBKGND:
        return 1;

        case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

        default:
         return DefWindowProc(hwnd,msg,wp,lp);

    }
    return 0;

}

int WINAPI main(HINSTANCE hinstance,HINSTANCE hprev,LPSTR pz,int cmdShow){
  
    TCHAR app_name[]=_T("skia App");
    TCHAR window_name[]=_T("skia +win32");
   WNDCLASSEX wc={};

   memset(&wc,0,sizeof(wc));
   wc.cbSize=sizeof(wc);
   wc.lpfnWndProc=WndProc;
   wc.hInstance=hinstance;
   wc.lpszClassName=app_name;
   wc.hbrBackground=nullptr;
   wc.hCursor=LoadCursor(nullptr,IDC_ARROW);

   if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}


   HWND hwnd=CreateWindowEx(
    WS_EX_CLIENTEDGE,app_name,window_name,
    WS_VISIBLE|WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,
    g_width,g_height,nullptr,nullptr,hinstance,nullptr
   );

   ShowWindow(hwnd,cmdShow);
   UpdateWindow(hwnd);

   if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}


   MSG msg;

   if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	while(GetMessage(&msg, NULL, 0, 0) > 0) { /* If no error is received... */
		TranslateMessage(&msg); /* Translate key codes to chars if present */
		DispatchMessage(&msg); /* Send it to WndProc */
	}
	return msg.wParam;
    
}




