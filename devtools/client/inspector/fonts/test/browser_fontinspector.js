/* vim: set ts=2 et sw=2 tw=80: */
/* Any copyright is dedicated to the Public Domain.
   http://creativecommons.org/publicdomain/zero/1.0/ */
"use strict";

requestLongerTimeout(2);

const TEST_URI = URL_ROOT + "browser_fontinspector.html";
const FONTS = [{
  name: "Ostrich Sans Medium",
  remote: true,
  url: URL_ROOT + "ostrich-regular.ttf",
  format: "truetype",
  cssName: "bar"
}, {
  name: "Ostrich Sans Black",
  remote: true,
  url: URL_ROOT + "ostrich-black.ttf",
  format: "",
  cssName: "bar"
}, {
  name: "Ostrich Sans Black",
  remote: true,
  url: URL_ROOT + "ostrich-black.ttf",
  format: "",
  cssName: "bar"
}, {
  name: "Ostrich Sans Medium",
  remote: true,
  url: URL_ROOT + "ostrich-regular.ttf",
  format: "",
  cssName: "barnormal"
}];

add_task(function* () {
  let { inspector, view } = yield openFontInspectorForURL(TEST_URI);
  ok(!!view, "Font inspector document is alive.");

  let viewDoc = view.document;

  yield testBodyFonts(inspector, viewDoc);
  yield testDivFonts(inspector, viewDoc);
  yield testShowAllFonts(inspector, viewDoc);
});

function isRemote(fontLiEl) {
  return fontLiEl.querySelectorAll(".font-format-url").length === 1;
}

function* testBodyFonts(inspector, viewDoc) {
  let lis = viewDoc.querySelectorAll("#all-fonts > li");
  is(lis.length, 5, "Found 5 fonts");

  for (let i = 0; i < FONTS.length; i++) {
    let li = lis[i];
    let font = FONTS[i];
    is(li.querySelector(".font-name").textContent, font.name,
       "font " + i + " right font name");

    is(isRemote(li), font.remote,
       "font " + i + " remote value correct");
    is(li.querySelector(".font-url").value, font.url,
       "font " + i + " url correct");
    is(li.querySelector(".font-format").hidden, !font.format,
       "font " + i + " format hidden value correct");
    is(li.querySelector(".font-format").textContent,
       font.format, "font " + i + " format correct");
    is(li.querySelector(".font-css-name").textContent,
       font.cssName, "font " + i + " css name correct");
  }

  // test that the bold and regular fonts have different previews
  let regSrc = lis[0].querySelector(".font-preview").src;
  let boldSrc = lis[1].querySelector(".font-preview").src;
  isnot(regSrc, boldSrc, "preview for bold font is different from regular");

  // test system font
  let localFontName = lis[4].querySelector(".font-name").textContent;
  let localFontCSSName = lis[4].querySelector(".font-css-name").textContent;

  // On Linux test machines, the Arial font doesn't exist.
  // The fallback is "Liberation Sans"
  ok((localFontName == "Arial") || (localFontName == "Liberation Sans"),
     "local font right font name");
  ok(!isRemote(lis[4]), "local font is local");
  ok((localFontCSSName == "Arial") || (localFontCSSName == "Liberation Sans"),
     "Arial", "local font has right css name");
}

function* testDivFonts(inspector, viewDoc) {
  let updated = inspector.once("fontinspector-updated");
  yield selectNode("div", inspector);
  yield updated;

  let lis = viewDoc.querySelectorAll("#all-fonts > li");
  is(lis.length, 1, "Found 1 font on DIV");
  is(lis[0].querySelector(".font-name").textContent,
     "Ostrich Sans Medium",
     "The DIV font has the right name");
}

function* testShowAllFonts(inspector, viewDoc) {
  info("testing showing all fonts");

  let updated = inspector.once("fontinspector-updated");
  viewDoc.querySelector("#font-showall").click();
  yield updated;

  // shouldn't change the node selection
  is(inspector.selection.nodeFront.nodeName, "DIV", "Show all fonts selected");
  let lis = viewDoc.querySelectorAll("#all-fonts > li");
  is(lis.length, 6, "Font inspector shows 6 fonts (1 from iframe)");
}
