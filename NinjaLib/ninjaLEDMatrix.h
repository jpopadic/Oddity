#pragma once

// the control class name
// used when actually building dialogs
#define	NINJALED_CLASSNAME	("ninjaLED")


/*===========================================================================*\
 | class that controls the custom control
 | a pointer to a local instance of one of these is stored in the host
 | window's userdata
\*===========================================================================*/

class cNinjaLED : public cNinjaControlBase
{
public:

	// storage for the image
	sUInt32		iWidth,iHeight;

	// mark that the parent window or hierarchy
	// has just changed size
	void sizeChanged();

	// draw the label
	void draw(HDC hdc);

	// static funtions callable via the control namespace
	static void registerThis(HINSTANCE hInstance);
	static void unregisterThis(HINSTANCE hInstance);
  void decodeFramebuffer( sUInt8 *framebuffer );

protected:

  COLORREF  m_LEDs[16][16];
};

// common creation and destruction functions for this control
// image viewer is basic to instance, but then needs several setup calls (above in class)
cNinjaLED *createNinjaLED(HWND hwHost);
void destroyNinjaLED(HWND hwHost);
cNinjaLED *getNinjaLED(HWND hwHost);
