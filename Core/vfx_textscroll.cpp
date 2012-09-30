/*
                 __    ___ __       
      ____  ____/ /___/ (_) /___  __
     / __ \/ __  / __  / / __/ / / /    [ LED matrix VFX core ] 
    / /_/ / /_/ / /_/ / / /_/ /_/ / 
    \____/\__,_/\__,_/_/\__/\__, /     [ john & harry ] [ 2012 ]
                           /____/   
*/


#include "oddity.h"
#include "vfx.h"

#include <string.h>

// ---------------------------------------------------------------------------------------------------------------------
TextScroller::TextScroller(const char* text)
{
  m_len = strlen(text);
  m_scroll = m_char = m_nextPixelCountdown = 0;
}

// ---------------------------------------------------------------------------------------------------------------------
void TextScroller::Render(byte* frame)
{
  char c1 = m_text[m_char];
  char c2 = m_text[(m_char + 1) % m_len];

  draw::FontGlyph(frame, c1, (int16_t)(-m_scroll), 0, Orange);
  draw::FontGlyph(frame, c2, (int16_t)(-m_scroll) + 16, 0, Orange);

  m_nextPixelCountdown --;
  if (m_nextPixelCountdown <= 0)
  {
    m_scroll ++;
    m_nextPixelCountdown = 1;

    if (m_scroll == 15)
    {
      m_char ++;
      if (m_char >= m_len)
        m_char = 0;

      m_scroll = 0;
    }
  }

}