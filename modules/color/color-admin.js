
(function () {
  function bind() {
    var pickers = document.querySelectorAll(".rp-color-picker");
    for (var i = 0; i < pickers.length; i++) {
      (function (picker) {
        var key = picker.getAttribute("data-key");
        var hex = document.querySelector(".rp-color-hex[data-key="" + key + ""]");
        if (!hex) return;
        picker.addEventListener("input", function () {
          hex.value = picker.value;
        });
        hex.addEventListener("change", function () {
          var v = hex.value.trim();
          if (/^#[0-9a-fA-F]{6}$/.test(v)) {
            picker.value = v;
          } else if (/^#[0-9a-fA-F]{3}$/.test(v)) {
            picker.value = "#" + v[1]+v[1]+v[2]+v[2]+v[3]+v[3];
          }
        });
      })(pickers[i]);
    }
  }
  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", bind);
  } else {
    bind();
  }
})();
