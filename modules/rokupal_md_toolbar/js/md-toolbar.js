/**
 * Insert Markdown around selection in the nearest textarea.
 */
(function () {
  function wrap(ta, before, after, placeholder) {
    if (!ta) return;
    ta.focus();
    var start = ta.selectionStart || 0;
    var end = ta.selectionEnd || 0;
    var val = ta.value;
    var selected = val.substring(start, end);
    if (!selected) selected = placeholder || '';
    var insert = before + selected + after;
    ta.value = val.substring(0, start) + insert + val.substring(end);
    var pos = start + before.length + selected.length;
    ta.selectionStart = start + before.length;
    ta.selectionEnd = pos;
    ta.focus();
  }

  function prefixLines(ta, prefix) {
    if (!ta) return;
    var start = ta.selectionStart || 0;
    var end = ta.selectionEnd || 0;
    var val = ta.value;
    // expand to full lines
    while (start > 0 && val.charAt(start - 1) !== '\n') start--;
    while (end < val.length && val.charAt(end) !== '\n') end++;
    var block = val.substring(start, end);
    var lines = block.split('\n');
    for (var i = 0; i < lines.length; i++) {
      if (lines[i].length) lines[i] = prefix + lines[i];
    }
    var out = lines.join('\n');
    ta.value = val.substring(0, start) + out + val.substring(end);
    ta.focus();
  }

  function findTextarea(btn) {
    var bar = btn.parentNode;
    if (!bar) return null;
    // next textarea sibling or in same form
    var n = bar.nextElementSibling;
    while (n) {
      if (n.tagName === 'TEXTAREA') return n;
      var t = n.querySelector && n.querySelector('textarea');
      if (t) return t;
      n = n.nextElementSibling;
    }
    var form = bar.closest ? bar.closest('form') : null;
    if (form) {
      var areas = form.querySelectorAll('textarea');
      if (areas.length) return areas[0];
    }
    return document.querySelector('textarea.rp-md-target') || document.querySelector('textarea');
  }

  function onClick(e) {
    var btn = e.target.closest ? e.target.closest('.rp-md-btn') : null;
    if (!btn) return;
    e.preventDefault();
    var md = btn.getAttribute('data-md');
    var ta = findTextarea(btn);
    if (!ta) return;
    switch (md) {
      case 'bold': wrap(ta, '**', '**', 'bold'); break;
      case 'italic': wrap(ta, '*', '*', 'italic'); break;
      case 'strike': wrap(ta, '~~', '~~', 'text'); break;
      case 'code': wrap(ta, '`', '`', 'code'); break;
      case 'link':
        var url = window.prompt('URL', 'https://');
        if (url) wrap(ta, '[', '](' + url + ')', 'text');
        break;
      case 'ul': prefixLines(ta, '- '); break;
      case 'ol': prefixLines(ta, '1. '); break;
      case 'quote': prefixLines(ta, '> '); break;
      case 'h2': prefixLines(ta, '## '); break;
    }
  }

  if (document.addEventListener) {
    document.addEventListener('click', onClick, false);
  }
})();
