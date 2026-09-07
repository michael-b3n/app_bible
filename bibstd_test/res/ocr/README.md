# OCR test images

Drop screenshots here. They are the source of the OCR data the `reference_ocr` test replays:
each image is run through the real tesseract engine once, and the recognized words, lines and
paragraphs are written next to it as a `.ocr` file. The test itself never reads an image, so it
stays fast and independent of the tesseract version installed.

Neither the images nor the captured data are committed, `.gitignore` keeps this folder empty
except for its documentation. The test skips its capture driven part when there is no `.ocr`
file, everything it checks with its own designed data still runs.

## What makes a good image

- A bible reference inside running text, not a reference standing on its own.
- Two to four lines of the surrounding paragraph, so the paragraph recognition has
  neighbouring lines to widen to.
- A reference broken across a line break is the most valuable case, e.g. "... steht in
  Johannes 3," at the end of one line and "16 und wurde ..." at the start of the next.
- A crop around the paragraph rather than a whole desktop.
- Text rendered the way the app really sees it: same scaling and font smoothing as the
  screen it was captured from.

## Capturing

Add the images, then run the capture. It reads every png, bmp, jpg and tif in this folder and
writes a `<name>.ocr` next to each of them:

```bash
build/bibstd_test/bibstd_test.exe "[.capture]"
```

The capture is hidden from ctest, because it needs the real tesseract engine and the tessdata
folder. Both paths come from the `BIBSTD_TEST_OCR_DIR` and `BIBSTD_TEST_TESSDATA_DIR` defines
in `bibstd_test/CMakeLists.txt`.
