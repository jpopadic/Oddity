
#include "ninjaFW.h"
#include "ninja.h"
#include "resource.h"

BOOL CALLBACK basicHostProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) 
  {
  case WM_INITDIALOG:
    {
      createNinjaFrame(GetDlgItem(hWnd,IDC_FRAME1),NULL,NJALIGN_LEFT,FALSE);

      createNinjaButton(GetDlgItem(hWnd,IDC_EXITODD),NULL,NJALIGN_CENTER,FALSE);
      setBaseHeavyStyle(GetDlgItem(hWnd,IDC_EXITODD),TRUE);

      createNinjaLED(GetDlgItem(hWnd,IDC_LED));

      SetTimer(hWnd,0x01,30,NULL);
    }
    break;

  case WM_TIMER:
    {
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

