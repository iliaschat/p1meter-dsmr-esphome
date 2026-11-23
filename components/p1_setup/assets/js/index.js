document.getElementById('logout').addEventListener('click', async () => {
    const button = document.getElementById('logout');
    button.disabled = true;
    try {
        const response = await fetch('/api/logout', {
            method: 'POST'
        });
        if (!response.ok) throw new Error('Could not sign out.');
        window.close();
        if (!window.closed) document.body.innerHTML = '<main style="margin:48px auto;padding:24px;max-width:520px;font-family:system-ui,sans-serif;color:#14231d"><h1>Signed out</h1><p>Your setup session has ended. You can close this tab.</p></main>'
    } catch (error) {
        button.disabled = false;
        window.alert(error.message)
    }
});

// ==== Hamburger‑menu toggle (added for the new UI) ==================================================
const hamburger = document.getElementById('hamburger-btn');
const dropdown  = document.getElementById('mobile-dropdown');

if (hamburger && dropdown) {
  hamburger.addEventListener('click', () => {
    dropdown.hidden = dropdown.hidden ? false : true;
    hamburger.setAttribute('aria-expanded', dropdown.hidden ? 'false' : 'true');
  });

  // Close dropdown when clicking outside
  document.addEventListener('click', (e) => {
    const insideHamburger = hamburger.contains(e.target);
    const insideDropdown  = dropdown.contains(e.target);
    if (insideHamburger === false && insideDropdown === false) {
      dropdown.hidden = true;
      hamburger.setAttribute('aria-expanded', 'false');
    }
  });

  // Dropdown item → trigger the original menu element
  dropdown.querySelectorAll('.dropdown-item').forEach(item => {
    item.addEventListener('click', (e) => {
      e.preventDefault();
      const targetId = item.getAttribute('data-target');
      const targetEl = document.getElementById(targetId);
      if (targetEl) { targetEl.click(); }
    });
  });
}