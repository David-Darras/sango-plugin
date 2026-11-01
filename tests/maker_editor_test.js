// Runs the rules of the Sango Maker Editor (tools/maker_editor/index.html) on
// the files of maker/data and of tests/maker_cases. The C++ reader reads the
// same files (tests/test_maker_files.cc): the two must find the same
// mistakes. In tests/maker_cases, a line that ends with the comment "ERR" has
// a mistake.
//
//   node tests/maker_editor_test.js
"use strict";
const fs = require("fs");
const path = require("path");
const vm = require("vm");

const root = path.join(__dirname, "..");
const editor = path.join(root, "tools", "maker_editor");

// The page has two scripts: names.js, then the code of the page. The test
// runs the code until the part of the files and the tabs, with a small
// document: it needs only the check functions.
const html = fs.readFileSync(path.join(editor, "index.html"), "utf8");
const scripts = [...html.matchAll(/<script>([\s\S]*?)<\/script>/g)].map((m) => m[1]);
const code = scripts[scripts.length - 1];
const end = code.indexOf("// The files ---");
if (end < 0) throw new Error("index.html: the part '// The files' is missing");

const element = () => ({ appendChild() {}, append() {}, set id(v) {}, set value(v) {} });
const context = {
  document: { getElementById: element, createElement: element },
  console,
};
vm.createContext(context);
vm.runInContext(fs.readFileSync(path.join(editor, "names.js"), "utf8")
  .replace("const SANGO_NAMES", "var SANGO_NAMES"), context);
vm.runInContext(code.slice(0, end) +
  "\nthis.checks = { encounters: checkEncounters, trainers: checkTrainers, " +
  "species: checkSpecies, story: checkStory, characters: checkCharacters };",
  context);

let failures = 0;
function checkFile(file, kind) {
  const text = fs.readFileSync(file, "utf8").replace(/\r/g, "");
  const found = new Set(context.checks[kind](text).map((e) => e.n));
  const expected = new Set();
  text.split("\n").forEach((line, i) => {
    if (/#\s*ERR\s*$/.test(line)) expected.add(i + 1);
  });
  const missing = [...expected].filter((n) => !found.has(n));
  const extra = [...found].filter((n) => !expected.has(n));
  const name = path.relative(root, file);
  if (missing.length === 0 && extra.length === 0) {
    console.log("ok   " + name);
    return;
  }
  failures++;
  console.log("FAIL " + name);
  if (missing.length) console.log("     no mistake on the lines " + missing.join(", "));
  if (extra.length) console.log("     unexpected mistakes on the lines " + extra.join(", "));
}

for (const kind of ["encounters", "trainers", "species", "story", "characters"]) {
  checkFile(path.join(root, "maker", "data", kind + ".txt"), kind);
  checkFile(path.join(root, "tests", "maker_cases", kind + ".txt"), kind);
}
console.log(failures === 0 ? "\nThe editor finds the same mistakes." : `\n${failures} files failed`);
process.exit(failures === 0 ? 0 : 1);
