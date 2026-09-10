(function () {
  function qs(s, ctx) { return (ctx || document).querySelector(s); }
  function qsa(s, ctx) { return Array.prototype.slice.call((ctx || document).querySelectorAll(s)); }

  function collectLayout() {
    var layout = {};
    qsa('.rp-blk-drop').forEach(function (drop) {
      var region = drop.getAttribute('data-region');
      layout[region] = [];
      qsa('.rp-blk-card', drop).forEach(function (card) {
        layout[region].push(card.getAttribute('data-id'));
      });
    });
    return layout;
  }

  function bind() {
    var dragged = null;
    qsa('.rp-blk-card').forEach(function (card) {
      card.addEventListener('dragstart', function (e) {
        dragged = card;
        card.classList.add('is-dragging');
        e.dataTransfer.effectAllowed = 'move';
        try { e.dataTransfer.setData('text/plain', card.getAttribute('data-id')); } catch (err) {}
      });
      card.addEventListener('dragend', function () {
        card.classList.remove('is-dragging');
        qsa('.rp-blk-drop').forEach(function (d) { d.classList.remove('drag-over'); });
        dragged = null;
      });
    });
    qsa('.rp-blk-drop').forEach(function (drop) {
      drop.addEventListener('dragover', function (e) {
        e.preventDefault();
        drop.classList.add('drag-over');
        e.dataTransfer.dropEffect = 'move';
      });
      drop.addEventListener('dragleave', function () {
        drop.classList.remove('drag-over');
      });
      drop.addEventListener('drop', function (e) {
        e.preventDefault();
        drop.classList.remove('drag-over');
        if (dragged) {
          drop.appendChild(dragged);
        }
      });
    });
    var form = qs('#rp-blk-form');
    if (form) {
      form.addEventListener('submit', function () {
        var input = qs('#rp-layout-json');
        if (input) {
          input.value = JSON.stringify(collectLayout());
        }
      });
    }
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', bind);
  } else {
    bind();
  }
})();
