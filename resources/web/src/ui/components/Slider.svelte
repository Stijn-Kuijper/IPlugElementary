<script lang="ts">
    import { SendMsgToIPlug, SendParameterValueToIPlug } from "../../main";
    import { gain } from "../stores/stores";

    $: sliderHeight = (($gain + 70) / 70) * 100;

    const mousedown = (ev: any) => {
        window.addEventListener('mousemove', mousemove);
        window.addEventListener('mouseup', mouseup);
    }

    const mousemove = (ev: any) => {
        if (ev.movementY === 0) return;
        let value: number = clamp($gain + (-0.1 * ev.movementY), -70, 0);
        value = (value + 70) / 70;
        SendParameterValueToIPlug(0, value);        
    }

    const mouseup = (ev: any) => {
        window.removeEventListener('mousemove', mousemove);
        window.removeEventListener('mouseup', mouseup);
    }

    const clamp = (num, min, max) => Math.min(Math.max(num, min), max);
</script>

<!-- svelte-ignore a11y-no-static-element-interactions -->
<div class="slider-wrapper" on:mousedown={mousedown}>
    <div class="slider" style="height: {sliderHeight}%"></div>
</div>
<div class="value">
    {$gain}dB
</div>

<style>
    .slider-wrapper {
        display: flex;
        flex-direction: column-reverse;
        position: relative;
        height: 256px;
        width: 16px;
        border: 2px solid black;
        cursor: pointer;
        user-select: none;
        -webkit-user-select: none;
    }

    .slider {
        display: block;
        width: 100%;
        background-color: gray;
        -webkit-user-select: none;
        cursor: pointer;
    }
</style>