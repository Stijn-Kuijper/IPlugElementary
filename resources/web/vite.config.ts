import { defineConfig } from 'vite'
import { svelte } from '@sveltejs/vite-plugin-svelte'
// This plugin will allow us to have all the code generated
//   by Vite to be inline within the `index.html` file.
// This way, we can open our entire application without the
//   need of a webserver!
import { viteSingleFile } from 'vite-plugin-singlefile'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [svelte(), viteSingleFile()],
})
