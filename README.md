<!DOCTYPE html>
<html>
<head>
<title>Page Title</title>
<script>
function init() {
	calc_alcohol();
}


function calc_alcohol() {
  var wort = document.getElementById("wort").value;
  var remaining_wort = document.getElementById("rem_wort").value;
  var realEVG = (0.81 * 100 * (wort - remaining_wort) / wort);
  var realExtract = (1 - (0.81 * (wort - remaining_wort) / wort)) * wort;
  var alcWeight = (realExtract - wort) / ((1.0665 * wort / 100) - 2.0665);
  var alcVol = alcWeight / 0.795;
  document.getElementById("real_evg").innerHTML = realEVG.toFixed(2);
  document.getElementById("real_extract").innerHTML = realExtract.toFixed(2);
  document.getElementById("volprocalc").innerHTML = alcVol.toFixed(2);
}


</script>
  <style>
    .box {
    	border-width: 1px;
    	border-style: solid;
    }

  </style>

</head>
<body onload="init()">

<div class="box">
        <h3>Alkoholgehalt des endvergorenen Bieres</h3>
        <form name="f_alcohol">
          <table style="border-style:none;">
            <tbody><tr>
            <td>Stammwürze (°Plato):</td>
            <td><input class="infield" type="number" name="wort" id="wort" value="12" onchange="javascript:calc_alcohol();"></td>
            </tr><tr>
            <td>Jungbier (°Plato):</td>
            <td><input class="infield" type="number" name="rem_wort" id="rem_wort"  value="2.5" onchange="javascript:calc_alcohol();"></td>
            </tr><tr>
            <td>Tatsächl. Endvergärungsgrad (%):</td>
            <td><b><div class="result" id="real_evg">...</div></b></td>
            </tr><tr>
            <td>Tatsächl. Restextrakt (%):</td>
            <td><b><div class="result" id="real_extract">...</div></b></td>
            </tr><tr>
            <td>Alkoholgehalt (Vol. %):</td>
            <td><b><div class="result" id="volprocalc">...</div></b></td>
            </tr>
          </tbody></table>
        </form>
  </div>
</body>
</html>
