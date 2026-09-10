/**
 * RokuPal front-end behaviours.
 */
(function (Drupal, $) {
  'use strict';
  Drupal.behaviors.rokupalUI = {
    attach: function (context) {
      // Mobile nav toggle if present
      var toggles = context.querySelectorAll ? context.querySelectorAll('[data-rp-nav-toggle]') : [];
      Array.prototype.forEach.call(toggles, function (btn) {
        if (btn.getAttribute('data-rp-bound')) return;
        btn.setAttribute('data-rp-bound', '1');
        btn.addEventListener('click', function () {
          var nav = document.querySelector('.rp-nav');
          if (nav) nav.classList.toggle('is-open');
        });
      });
    }
  };
})(window.Drupal || {}, window.jQuery);
