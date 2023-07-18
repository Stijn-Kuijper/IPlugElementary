# Svelte + TS + Vite
For the JS code, this project uses a combination of Svelte, TypeScript and Vite. These choices are personal favourites and you should be able to use different frameworks/bundlers/etc. if you want.

# Installation
With `npm` installed, install the required packages using
```zsh
npm install
```
After this, you can run the development server using
```zsh
npm run dev
```
Or compile your website into a single `index.html` file using
```zsh
npm run build
```
In case you want to use the compiled, static website, be sure to update the constructor of the Plugin instance within the main `.cpp` file of the project.

# Static HTML file vs. Webserver
Normally, when using Vite to build your application, Vite will output a `.js` file and link it as a module within the generated `index.html` file. This means that you can only open your application using a webserver: when trying to open it as a file, you will run into `CORS` issues.
To combat this, this project uses the [`vite-plugin-singlefile`](https://www.npmjs.com/package/vite-plugin-singlefile) plugin, which will place everything inline within the `index.html` file, allowing you to open your application without the need of a webserver. Keep in mind that this brings in some limitations. A list of known limitations can be found at the bottom of the [`vite-plugin-singlefile` npm page](https://www.npmjs.com/package/vite-plugin-singlefile).

If you want to use something else than Vite, make sure there is an option to have a single `index.html` file with inline JS code to prevent headaches of having to open a webserver within iPlug.
