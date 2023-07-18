<script lang="ts">
    import Button from "./ui/components/Button.svelte";
    import Slider from "./ui/components/Slider.svelte";
    import Meter from "./ui/components/Meter.svelte";

    import { meterLeft, meterRight } from "./ui/stores/stores";
</script>

<main style="display: flex;">
    <div>
        <Slider />
        <Button />
    </div>
    <Meter value={$meterLeft} />
    <Meter value={$meterRight} />
</main>

<style>

</style>
