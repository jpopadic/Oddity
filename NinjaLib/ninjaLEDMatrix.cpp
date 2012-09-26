/*===========================================================================*\
|
| Ninja Custom Control Library
|
| Copyright© 2000-2001 Harry Denholm. All Rights Reserved.
|===========================================================================*
|
| Image edit and viewing chamber
|
| $Author: Harry Denholm $ 
| $Revision: 1 $
| $Date: 22/05/01 23:18 $ 
| $Archive: /Ninja/Controls/ninjaLabel.cpp $
|
\*===========================================================================*/


#include "ninjaFW.h"
#include "ninja.h"
#include "GDITools.h"

static COLORREF ledColors[4][4];

/*===========================================================================*\
| register the control with windows, bound to the passed app instance handle
\*===========================================================================*/

void cNinjaLED::registerThis(HINSTANCE hInstance)
{
  WNDCLASS winClass;

  winClass.style			= CS_DBLCLKS | CS_OWNDC | 
    CS_HREDRAW | CS_VREDRAW;
  winClass.lpfnWndProc	= ninjaControlProc;
  winClass.cbClsExtra		= 0;
  winClass.cbWndExtra		= 0;
  winClass.hInstance		= hInstance;
  winClass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
  winClass.hCursor		= LoadCursor(NULL, IDC_ARROW);
  winClass.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);
  winClass.lpszMenuName	= NULL; 
  winClass.lpszClassName	= NINJALED_CLASSNAME;

  // register the window class, or fail and warn the user
  if(!RegisterClass(&winClass))
    MessageBox(NULL,"Ninja Error:\n\nLED View Class Unregistered!","Error",MB_OK);
}


/*===========================================================================*\
| register the control with windows, bound to the passed app instance handle
\*===========================================================================*/

void cNinjaLED::unregisterThis(HINSTANCE hInstance)
{
  // unbind the control from the app instance
  UnregisterClass(NINJALED_CLASSNAME,hInstance);
}


/*===========================================================================*\
| size of parent or hierarchy has changed - update
| any dimensions based caches
\*===========================================================================*/

void cNinjaLED::sizeChanged()
{
  if(!hwHost) return;
  GetClientRect(hwHost,&hostDim);
}


/*===========================================================================*\
| draw the control onto a device context
\*===========================================================================*/

void cNinjaLED::draw(HDC hdc)
{
  // get current GDI data setup
  sNinjaGDI	*curNGDI = getActiveNGDI();
  if(!curNGDI) return;


  RECT checkTag;
  checkTag.top = 0;
  checkTag.left = 0;
  checkTag.right = checkTag.left + (16 * 18) + 3 + 3;
  checkTag.bottom = checkTag.top + (16 * 18) + 3 + 3;

  // create double buffer storage
  HDC hdcN	= CreateCompatibleDC(hdc);
  HBITMAP img = CreateCompatibleBitmap(hdc,checkTag.right + 1,checkTag.bottom + 1);
  SelectObject(hdcN,img);

  njColourRect(hdcN, &checkTag, RGB(0,0,0));

  for (int y=0; y<16; y++)
  {
    for (int x=0; x<16; x++)
    {
      // draw the checkbox tag
      checkTag.top = 2 + (y * 18);
      checkTag.left = 2 + (x * 18);
      checkTag.right = checkTag.left + 16;
      checkTag.bottom = checkTag.top + 16;
      njColourRect(hdcN, &checkTag, m_LEDs[x][y]);
    }
  }

  // copy the buffer, and delete the DC
  BitBlt(hdc,0,0,checkTag.right + 1,checkTag.bottom + 1,hdcN,0,0,SRCCOPY);
  DeleteObject(img);
  DeleteDC(hdcN);
}

void cNinjaLED::decodeFramebuffer( sUInt8 *framebuffer )
{
  for (int y=0; y<16; y++)
  {
    for (int x=0; x<16; x++)
    {
      sUInt8 pixel = framebuffer[(y * 16) + x];
      sUInt8 red   = pixel & 0x0f;
      sUInt8 green = (pixel >> 4) & 0x0f;

      m_LEDs[x][y] = ledColors[red][green];
    }
  }
}


/*===========================================================================*\
| create a new instance of the control's manager class and
| insert it into the host's userdata entry
\*===========================================================================*/

cNinjaLED *createNinjaLED(HWND hwHost)
{
  // create new manager
  cNinjaLED *cLED = new cNinjaLED();
  cLED->resetBase();

  cLED->hwHost = hwHost;

  for (int r=0; r<4; r++)
  {
    for (int g=0; g<4; g++)
    {
      int led_red = 40 + (r * 70);
      int led_green = 40 + (g * 70);

      ledColors[r][g] = RGB(led_red, led_green, 0);
    }
  }

  // bind to the window
  SetWindowLong(hwHost,GWL_USERDATA,(LONG)cLED);

  // init the control's window proc
  SendMessage(hwHost,WM_NINJA_INIT,0,0);

  return cLED;
}


/*===========================================================================*\
| tidy up a control manager class and remove it from a host window
\*===========================================================================*/

void destroyNinjaLED(HWND hwHost)
{
  // get manager and whack it
  cNinjaLED *cLED = (cNinjaLED*)GetWindowLong(hwHost,GWL_USERDATA);
  delete cLED;

  // delete userdata
  SetWindowLong(hwHost,GWL_USERDATA,NULL);
}


/*===========================================================================*\
| get the control manager pointer from a host
\*===========================================================================*/

cNinjaLED *getNinjaLED(HWND hwHost)
{
  cNinjaLED *cLED = (cNinjaLED*)GetWindowLong(hwHost,GWL_USERDATA);
  return cLED;
}
