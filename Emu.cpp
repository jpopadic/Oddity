
#include "ninjaFW.h"
#include "ninja.h"
#include "resource.h"

#include "oddity.h"

cNinjaLED *LEDdisplay = NULL;
byte framebuffer[FRAME_SIZE];

BOOL CALLBACK basicHostProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) 
  {
  case WM_INITDIALOG:
    {
      createNinjaFrame(GetDlgItem(hWnd,IDC_FRAME1),NULL,NJALIGN_LEFT,FALSE);

      createNinjaButton(GetDlgItem(hWnd,IDC_EXITODD),NULL,NJALIGN_CENTER,FALSE);
      setBaseHeavyStyle(GetDlgItem(hWnd,IDC_EXITODD),TRUE);

      LEDdisplay = createNinjaLED(GetDlgItem(hWnd,IDC_LED), FRAME_WIDTH);

      SetTimer(hWnd,0x01,16,NULL);

      memset(framebuffer, 0, sizeof(framebuffer));
      oddity_init();
    }
    break;

  case WM_TIMER:
    {
      memset(framebuffer, 0, sizeof(framebuffer));

      hw_inputs inputs;
      memset(&inputs, 0, sizeof(inputs));

      oddity_tick(framebuffer, inputs);

      LEDdisplay->decodeFramebuffer(framebuffer);
      // decode framebuffer

      InvalidateRect(LEDdisplay->hwHost, 0, FALSE);
    }
    break;


  case WM_NINJA_BUTTON:
    {
      switch(lParam)
      {
      case IDC_EXITODD:
        EndDialog(hWnd,0);
        break;
      }
    }
    break;

  default:
    return FALSE;
  }
  return TRUE;
}


int WINAPI WinMain(	HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR lpcmdline,
  int ncmdshow)
{
  InitNinjaControls(hInstance,NULL);

  DialogBox(
    hInstance,
    MAKEINTRESOURCE(IDD_HOST),
    GetDesktopWindow(),
    basicHostProc);

  return 0;
}

