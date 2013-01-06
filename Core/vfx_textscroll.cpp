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
TextScroller::TextScroller()
{
  m_len = 0;
}

// ---------------------------------------------------------------------------------------------------------------------
void TextScroller::Init(const char* text)
{
  m_wave = 0.0f;
  m_text = text;
  m_len = strlen(text);
  m_scroll = m_char = m_nextPixelCountdown = 0;
}

// ---------------------------------------------------------------------------------------------------------------------
bool TextScroller::Render(byte* frame, ColourChoice cc)
{
  int nextChar = (m_char + 1) % m_len;

  char c1 = m_text[m_char];
  char c2 = m_text[nextChar];

  Fix16 f1 = (int16_t)m_char;
  Fix16 f2 = (int16_t)nextChar;
  Fix16 f3, f4;

  f3 = Fix16::sin(m_wave + f1) * 3.0f;
  f4 = Fix16::sin(m_wave + f2) * 3.0f;

  f1 = Fix16::sin(m_wave + f1) * 4.0f;
  f2 = Fix16::sin(m_wave + f2) * 4.0f;


  draw::FontGlyph16x16(frame, c1, (int16_t)(-m_scroll), f3.asInt(), Red);
  draw::FontGlyph16x16(frame, c2, (int16_t)(-m_scroll) + 16, f4.asInt(), Red);

  draw::FontGlyph16x16(frame, c1, (int16_t)(-m_scroll) + 1, f1.asInt(), cc);
  draw::FontGlyph16x16(frame, c2, (int16_t)(-m_scroll) + 15, f2.asInt(), cc);

  m_wave += 0.2f;

  m_nextPixelCountdown --;
  if (m_nextPixelCountdown <= 0)
  {
    m_scroll ++;
    m_nextPixelCountdown = 1;

    if (m_scroll == 15)
    {
      m_scroll = 0;

      m_char ++;
      if (m_char >= m_len)
      {
        m_char = 0;
        return false;
      }
    }
  }

  return true;
}