(function () {
  function bind() {
    var wrap = document.getElementById("rp-comment-form-wrap");
    var btn = document.getElementById("rp-comment-toggle");
    if (!wrap || !btn) return;
    btn.addEventListener("click", function () {
      var collapsed = wrap.classList.contains("is-collapsed");
      if (collapsed) {
        wrap.classList.remove("is-collapsed");
        btn.setAttribute("aria-expanded", "true");
        btn.textContent = btn.getAttribute("data-open-label") || "Hide comment form";
      } else {
        wrap.classList.add("is-collapsed");
        btn.setAttribute("aria-expanded", "false");
        btn.textContent = btn.getAttribute("data-closed-label") || "Write a comment";
      }
    });
    btn.setAttribute("data-closed-label", btn.textContent);
    btn.setAttribute("data-open-label", "Hide comment form");
  }
  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", bind);
  } else {
    bind();
  }
})();
