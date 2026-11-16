# Lincstation 2D (OpenGL 3.3 core, C++17)

2D simulacija pametnog sefa Lincsafe Lincstation 3.0 (samostalna tema). Specifikacija: `claude/lincstation-2d-spec.md` u projektu.

## Build

```
cmake -S . -B build
cmake --build build
cd build && ./lincstation          # Windows: lincstation.exe (shaders/ i assets/ se kopiraju pored exe-a)
ctest                              # ili ./machine_test: headless testovi logike
```
CMake sam preuzima GLFW 3.4, glm 1.0.1 i FreeType 2.13.3. GLAD i stb_image su u `third_party/`.
Linux: potrebni X11 dev paketi (Arch: `libxrandr libxinerama libxcursor libxi`).

## Ime, prezime, indeks (overlay)

```
python3 tools/make_assets.py --name Mihajlo --surname Milojević --index "SV 00/2024"
```
(traži Pillow; ponovo generiše i kursore). Zatim ponovi build.

## Upotreba

- **Fill up**: dugme, zatim prevuci novčanice/kovanice u slotove; **Done** za kraj.
- **Payment**: u IDLE kucaj iznos (cifre, `.` ili `,`, Backspace, Enter), pa **Payment**. Ubaci novac; kusur se uzima klikom na otvor.
- **Unlock top hatch**: klik na gornji blok ga podiže/spušta. **Open coin drawer**: klik na fioku je uvlači (i zaključava).
- **CIT procedure**: klik na srednja vrata → prevuci kasetu u CIT bag → prevuci novu kasetu u mašinu → klik na vrata.
- **Part labels** (gore levo): prikazuje imena delova.
- **Esc** izlazi uvek.

## Checklist zahteva

| Zahtev | Gde |
|---|---|
| OpenGL ≥ 3.3, programabilni pajplajn | `main.cpp` (core profile), `shaders/` |
| Šejderi, uniformi | `quad.vert/frag`: uPos, uSize, uColor, uAlpha, uFlipX, uTexture, uMode, uShape, uRadius, uXform |
| Providne boje/teksture | svetla (pulsiranje), labele, overlay, hover |
| Miš i tastatura | drag-and-drop, klikovi, kucanje iznosa |
| Kursor sa slikom, tematski | `App::pickCursor()`, 6 PNG kursora |
| Tekst/brojevi | `gfx/Text` (FreeType) |
| 75 FPS | `main.cpp` |
| Full screen | `glfwCreateWindow(..., monitor, ...)` |
| Esc uvek | `App::onKey` |
| Poluprovidna tekstura sa imenom | `assets/tex/name_overlay.png` |
| Konstantna brzina | `core/Anim.h`, `ui/DragSystem.cpp` |

## Struktura

```
src/main.cpp          prozor, petlja, limiter
src/App.*             događaji, hit-testing, crtanje HUD-a
src/gfx/              Shader, Texture, Renderer (SDF oblici), Text (FreeType)
src/model/            Machine (stanja, novac, CIT), Cashier (dugmad, kucanje), Money
src/ui/               Layout (sve koordinate 960x540), MachineView, DragSystem
tests/machine_test.cpp  60 provera logike bez OpenGL-a
```
