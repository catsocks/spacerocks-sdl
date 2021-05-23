#!/usr/bin/env node
const fs = require('fs');
const path = require('path');
const yargs = require('yargs');
const Inliner = require('inliner');
const cheerio = require('cheerio');

const argv = yargs
    .scriptName('build-manual')
    .usage('Builds the Spacerocks HTML manual file')
    .help()
    .alias('help', 'h')
    .version(false)
    .options({
      'input': {
        alias: 'i',
        description: 'path to the website index.html file',
        requiresArg: true,
        required: true,
      },
      'output-folder': {
        alias: 'o',
        description: 'path to the folder the manual should be saved in',
        requiresArg: true,
        required: true,
      },
    })
    .argv;

new Inliner(argv.input, (err, html) => {
  if (err) {
    console.error(err);
    process.exit(1);
  }

  const $ = cheerio.load(html);
  $('script').remove();
  $('[class="no-print"]').remove();

  fs.writeFileSync(path.join(argv.outputFolder, 'Spacerocks.html'), $.html());
});
