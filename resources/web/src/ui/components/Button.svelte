<script lang="ts">
    import { SendParameterValueToIPlug } from "../../main";
    import { bypass } from "../stores/stores";

    const bypassGain = () => {
        SendParameterValueToIPlug(1, ($bypass ? 0 : 1));
    }
</script>

<!-- svelte-ignore a11y-click-events-have-key-events -->
<!-- svelte-ignore a11y-no-static-element-interactions -->
<div class:pressed={$bypass} class="button" on:click={bypassGain}>
    Bypass gain
</div>

<style>
    .button {
        display: block;
        padding: 8px;
        border: 1px solid black;
        width: max-content;
        cursor: pointer;
    }

    .pressed {
        color: white;
        background-color: grey;
    }
</style>