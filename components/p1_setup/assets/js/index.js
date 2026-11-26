const $ = id => document.getElementById(id);
$('logout').addEventListener('click', async () => {
    const button = $('logout');
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
const hamburger = $('hamburger-btn');
const dropdown = $('mobile-dropdown');

if (hamburger && dropdown) {
    hamburger.addEventListener('click', () => {
        dropdown.hidden = dropdown.hidden ? false : true;
        hamburger.setAttribute('aria-expanded', dropdown.hidden ? 'false' : 'true');
    });

    // Close dropdown when clicking outside
    document.addEventListener('click', (e) => {
        const insideHamburger = hamburger.contains(e.target);
        const insideDropdown = dropdown.contains(e.target);
        if (insideHamburger === false && insideDropdown === false) {
            dropdown.hidden = true;
            hamburger.setAttribute('aria-expanded', 'false');
        }
    });
}

// Communication submenu toggle
const commToggle = $('menu-communication');
const submenu = $('submenu-communication');
commToggle.addEventListener('click', ev => {
    if (submenu.contains(ev.target)) return; // clicking a submenu item must not toggle the submenu
    const expanded = commToggle.getAttribute('aria-expanded') === 'true';
    commToggle.setAttribute('aria-expanded', String(!expanded));
    submenu.hidden = expanded;
});

// ==== Generic modal ==================================================
const menuItemsWithModal = document.querySelectorAll('.menu-item[data-modal]');

menuItemsWithModal.forEach(function(item) {
    item.addEventListener('click', function(event) {
        event.preventDefault();

        const url = item.getAttribute('data-modal');
        const title = item.getAttribute('data-modal-title') ?? '';
        const subtitle = item.getAttribute('data-modal-subtitle') ?? '';

        fetch(url)
            .then(response => {
                if (!response.ok) throw new Error(`HTTP ${response.status}`);
                return response.text();
            })
            .then(html => {
                showGenericModal(html, title, subtitle);
            })
            .catch(err => console.error('Failed to load:', err));
    });
});

function showGenericModal(content, title, subtitle) {
    $('modal-title').innerHTML = title;
    $('modal-subtitle').innerHTML = subtitle;
    $('generic-modal').classList.add('open');

    // The fetched HTML is a complete document. Only its <body> content belongs
    // inside #modal-content; <head> (styles) is extracted and re-injected so
    // page-specific CSS is applied within the modal.
    const parser = new DOMParser();
    const doc = parser.parseFromString(content, 'text/html');
    $('modal-content').innerHTML = doc.body.innerHTML;
    Array.from(doc.querySelectorAll('style')).forEach(styleEl => {
        const clone = document.createElement('style');
        clone.textContent = styleEl.textContent;
        $('modal-content').appendChild(clone);
    });

    executeScripts($('modal-content'));
}

$('modal-close').addEventListener('click', () => {
    $('generic-modal').classList.remove('open')
});
$('generic-modal').addEventListener('click', event => {
    if (event.target === $('generic-modal')) $('generic-modal').classList.remove('open')
});

// ==== Scripts ==================================================
function executeScripts(container) {
    // Re-create inline <script> elements so they execute in the main document.
    // Scripts injected via innerHTML are inert by design.
    const inlineScripts = Array.from(container.querySelectorAll('script:not([src])'));
    inlineScripts.forEach(oldScript => {
        const script = document.createElement('script');
        Array.from(oldScript.attributes).forEach(attr => {
            if (attr.name !== 'src') script.setAttribute(attr.name, attr.value);
        });
        script.textContent = oldScript.textContent;
        oldScript.parentNode.replaceChild(script, oldScript);
    });

    // Re-create external <script src> elements so they load and execute.
    const externalScripts = Array.from(container.querySelectorAll('script[src]'));
    externalScripts.forEach(oldScript => {
        const script = document.createElement('script');
        Array.from(oldScript.attributes).forEach(attr => script.setAttribute(attr.name, attr.value));
        oldScript.parentNode.replaceChild(script, oldScript);
    });
}

/*function executeScripts(container) {
    const scripts = Array.from(container.querySelectorAll('script'));

    scripts.forEach(oldScript => {
        const newScript = document.createElement('script');

        // Copy all attributes (src, type, async, data-*, etc.)
        Array.from(oldScript.attributes).forEach(attr => {
            newScript.setAttribute(attr.name, attr.value);
        });
        // Copy inline content
        newScript.textContent = oldScript.textContent;
        // Replace the inert script with a live one
        oldScript.parentNode.replaceChild(newScript, oldScript);
    });
}*/

/*function aa (container, html){
  const parser = new DOMParser();
    const doc = parser.parseFromString(html, 'text/html');
    // Extract scripts
    const scripts = Array.from(doc.querySelectorAll('script')).map(s => ({
        src: s.src || null,
        type: s.type || '',
        text: s.textContent
    }));
    doc.querySelectorAll('script').forEach(s => s.remove());
    // Extract styles
    const styles = Array.from(doc.querySelectorAll('style')).map(s => s.textContent);
    doc.querySelectorAll('style').forEach(s => s.remove());
    // Insert content
    const container = document.getElementById('modal-content');
    container.innerHTML = doc.body.innerHTML;
    // Inject styles
    styles.forEach(css => {
        const styleEl = document.createElement('style');
        styleEl.textContent = css;
        container.appendChild(styleEl);
    });

    // Execute scripts
    scripts.forEach(s => {
        const el = document.createElement('script');
        if (s.type) el.type = s.type;
        if (s.src) {
            el.src = s.src;
            document.body.appendChild(el);
        } else {
            el.textContent = s.text;
            container.appendChild(el);
        }
    });
}*/