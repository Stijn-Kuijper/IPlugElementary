<script lang="ts">
    export let value;

    $: meterHeight = value * 100;
</script>

<div>
<!-- svelte-ignore a11y-no-static-element-interactions -->
    <div class="meter-wrapper">
        <div class="meter" style="height: {meterHeight}%"></div>
    </div>
    <div class="value">
        {value}
    </div>
</div>


<style>
    .meter-wrapper {
        display: flex;
        flex-direction: column-reverse;
        position: relative;
        height: 256px;
        width: 8px;
        border: 1px solid black;
        cursor: pointer;
        user-select: none;
        -webkit-user-select: none;
    }

    .meter {
        display: block;
        width: 100%;
        background-color: green;
        -webkit-user-select: none;
        cursor: pointer;
    }
</style>