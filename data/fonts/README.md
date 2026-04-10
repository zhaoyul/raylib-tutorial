# Chinese Font Setup For Raygui Chapters

`chapters/07-raygui-basics` and `chapters/08-raygui-advanced` try to load Chinese fonts from `data/fonts/` when you run them from the repository root.

## Supported filenames

Place any one of these files in this directory:

- `AlibabaPuHuiTi-Regular.otf`
- `NotoSansSC-Regular.otf`
- `NotoSansCJKsc-Regular.otf`

## Recommended quick download

```bash
mkdir -p data/fonts

curl -L -o data/fonts/AlibabaPuHuiTi-Regular.otf \
  "https://cdn.jsdelivr.net/npm/alibaba-puhuiti-2/Alibaba-PuHuiTi-Regular/Alibaba-PuHuiTi-Regular.otf"
```

## Notes

- If no supported font is present, the Raygui chapters fall back to English text.
- Run the chapter binary from the repository root if you want the relative path `data/fonts/...` to resolve cleanly:

```bash
./build/bin/chapters/chapter07_raygui_basics
./build/bin/chapters/chapter08_raygui_advanced
```
