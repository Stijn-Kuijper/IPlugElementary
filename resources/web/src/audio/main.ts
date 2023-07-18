import { Renderer, el } from '@elemaudio/core';

export function reconcile(core: Renderer, gain: number, bypass: boolean): void {
    if (core) {
        if (bypass) {
            core.render(el.in({channel: 0}), el.in({channel: 1}));
        } else {
            core.render(
                el.mul(el.db2gain(el.const({value: gain, key: "gain"})), el.in({channel: 0})),
                el.mul(el.db2gain(el.const({value: gain, key: "gain"})), el.in({channel: 1})),
            )
        }
    }
}