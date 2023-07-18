import { writable } from 'svelte/store';

export const gain = writable(-35);
export const bypass = writable(false);

export const meterLeft = writable(0);
export const meterRight = writable(0);