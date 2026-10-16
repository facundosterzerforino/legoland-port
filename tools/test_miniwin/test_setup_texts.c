/* test_setup_texts: the setup window's texts in every language (port/port_setup_win.c) measured with Windows' message
 * font, against the room the window gives them: the intro (448 x 80), the language label (260), the buttons
 * (109, less the button's own margins), and every message's line count. Nothing is shown.
 *
 *   tools/test_miniwin/test_setup_texts.exe */
#include "../../port/port_setup_win.c"

void PortTrace(const char *fmt, ...) {
    (void)fmt;
}

int main(void) {
    NONCLIENTMETRICSW m;
    HFONT font;
    HDC dc = CreateCompatibleDC(NULL);
    int problems = 0;
    int l, t;

    memset(&m, 0, sizeof(m));
    m.cbSize = sizeof(m);
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(m), &m, 0);
    font = CreateFontIndirectW(&m.lfMessageFont);
    SelectObject(dc, font);
    printf("message font: %ls, %ld\n", m.lfMessageFont.lfFaceName, m.lfMessageFont.lfHeight);
    for (l = 0; l < LANGUAGES; l++) {
        static const int buttons[] = {T_INSTALL, T_CHOOSE_ISO, T_CHOOSE_FOLDER, T_QUIT, T_CANCEL};
        RECT r = {0, 0, 448, 0};
        SIZE size;
        int widest = 0;

        language = l;
        DrawTextW(dc, Wide(T(T_INTRO)), -1, &r, DT_WORDBREAK | DT_CALCRECT);
        for (t = 0; t < 5; t++) {
            GetTextExtentPoint32W(dc, Wide(T(buttons[t])), lstrlenW(Wide(T(buttons[t]))), &size);
            if (size.cx > widest) {
                widest = size.cx;
            }
        }
        GetTextExtentPoint32W(dc, Wide(T(T_LANGUAGE)), lstrlenW(Wide(T(T_LANGUAGE))), &size);
        printf("%-10s intro %3ld px high (room 80)  widest button text %3d px (room 97)  label %3ld px (room 260)%s\n",
            setup_texts[l].name, r.bottom, widest, size.cx,
            (r.bottom > 80 || widest > 97 || size.cx > 260) ? "  DOESN'T FIT" : "");
        problems += r.bottom > 80 || widest > 97 || size.cx > 260;
        for (t = 0; t < T_COUNT; t++) {
            if (setup_texts[l].text[t] == NULL || setup_texts[l].text[t][0] == '\0') {
                printf("  missing text %d\n", t);
                problems++;
            }
        }
    }
    printf(problems ? "%d problem(s)\n" : "everything fits\n", problems);
    return problems != 0;
}
