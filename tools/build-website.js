#!/usr/bin/env node
const yargs = require('yargs');
const fs = require('fs-extra');
const path = require('path');

const argv = yargs
    .scriptName('build-website')
    .usage('Builds the Spacerocks website')
    .help()
    .alias('help', 'h')
    .version(false)
    .options({
      'assets-folder': {
        alias: 'a',
        description: 'path to the folder with the web assets',
        requiresArg: true,
        required: true,
      },
      'build-folder': {
        alias: 'b',
        description: 'path to the folder with the build of the game',
        requiresArg: true,
        required: true,
      },
      'output-folder': {
        alias: 'o',
        description: 'path to the folder to build the website in',
        requiresArg: true,
        required: true,
      },
      'url': {
        description: 'URL of the website (can also be set with the "URL" env.' +
          ' var.)',
        requiresArg: true,
      },
      'force-url-https': {
        description: 'replace http:// with https:// in the URL',
        type: 'boolean',
      },
    })
    .argv;

if (!argv.url) {
  argv.url = process.env.URL || 'http://localhost';
}

if (argv.forceUrlHttps) {
  if (argv.url.startsWith('http://')) {
    argv.url.replace('http://', 'https://');
  }
}

fs.mkdirSync(argv.outputFolder, {recursive: true});

fs.copySync(argv.assetsFolder, argv.outputFolder);

for (const f of ['game.html', 'game.js', 'game.wasm', 'game.data']) {
  fs.copySync(path.join(argv.buildFolder, f), path.join(argv.outputFolder, f));
}

for (const f of ['index.html', 'game.html']) {
  const filePath = path.join(argv.outputFolder, f);
  const contents = fs.readFileSync(filePath).toString();
  fs.writeFileSync(filePath, contents.replace(/{{url}}/g, argv.url));
}
