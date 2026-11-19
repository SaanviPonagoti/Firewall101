/* Shared: icons, navbar, footer, report storage (browser localStorage prototype) */
const ICONS = {
  shield: '<path d="M12 2l8 3v6c0 5-3.4 9.3-8 11-4.6-1.7-8-6-8-11V5z"/><path d="M9 12l2 2 4-4"/>',
  cycle: '<path d="M21 12a9 9 0 0 1-15.5 6.2"/><path d="M3 12A9 9 0 0 1 18.5 5.8"/><path d="M18 2v4h-4"/><path d="M6 22v-4h4"/>',
  bug: '<rect x="8" y="8" width="8" height="12" rx="4"/><path d="M12 8V5M8 12H4M20 12h-4M8 17l-3 2M16 17l3 2M9 5l1.5 1.5M15 5l-1.5 1.5"/>',
  book: '<path d="M4 4h10a4 4 0 0 1 4 4v12H8a4 4 0 0 1-4-4z"/><path d="M8 8h6M8 12h6"/>',
  users: '<circle cx="9" cy="8" r="3.5"/><path d="M2 21c0-4 3-6.5 7-6.5s7 2.5 7 6.5"/><path d="M16 4.5a3.5 3.5 0 0 1 0 7M18 15c2.5.8 4 2.7 4 6"/>',
  folder: '<path d="M3 6a2 2 0 0 1 2-2h4l2 2h8a2 2 0 0 1 2 2v10a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/>',
  alert: '<path d="M12 3l10 18H2z"/><path d="M12 10v5M12 18v.01"/>',
  mail: '<rect x="3" y="5" width="18" height="14" rx="2"/><path d="M3 7l9 6 9-6"/>',
  lock: '<rect x="5" y="11" width="14" height="10" rx="2"/><path d="M8 11V7a4 4 0 0 1 8 0v4"/>',
  key: '<circle cx="8" cy="15" r="4"/><path d="M11 12l9-9M16 7l3 3"/>',
  db: '<ellipse cx="12" cy="5" rx="8" ry="3"/><path d="M4 5v14c0 1.7 3.6 3 8 3s8-1.3 8-3V5M4 12c0 1.7 3.6 3 8 3s8-1.3 8-3"/>',
  phone: '<rect x="7" y="2" width="10" height="20" rx="2"/><path d="M11 18h2"/>',
  globe: '<circle cx="12" cy="12" r="9"/><path d="M3 12h18M12 3c3 3 3 15 0 18M12 3c-3 3-3 15 0 18"/>',
  wifi: '<path d="M2 9a15 15 0 0 1 20 0M5 12.5a11 11 0 0 1 14 0M8.5 16a6 6 0 0 1 7 0M12 19.5v.01"/>',
  edit: '<path d="M4 20h4L19 9l-4-4L4 16z"/>',
  update: '<path d="M21 12a9 9 0 1 1-3-6.7"/><path d="M21 4v5h-5"/>',
  backup: '<path d="M7 18a5 5 0 0 1-.5-10A6 6 0 0 1 18 9a4.5 4.5 0 0 1-1 9z"/><path d="M12 11v6M9 14l3-3 3 3"/>',
  help: '<circle cx="12" cy="12" r="9"/><path d="M9.5 9.5a2.5 2.5 0 1 1 3.5 2.3c-.7.4-1 .9-1 1.7M12 17v.01"/>',
  pin: '<path d="M12 22s7-6.5 7-12a7 7 0 0 0-14 0c0 5.5 7 12 7 12z"/><circle cx="12" cy="10" r="2.5"/>',
  check: '<path d="M5 12l5 5 9-10"/>',
  file: '<path d="M6 2h8l5 5v15H6z"/><path d="M14 2v5h5M9 13h7M9 17h7"/>',
  dash: '<rect x="3" y="3" width="8" height="10" rx="1"/><rect x="13" y="3" width="8" height="6" rx="1"/><rect x="3" y="15" width="8" height="6" rx="1"/><rect x="13" y="11" width="8" height="10" rx="1"/>'
};
function icon(name, size = 24) {
  return `<svg width="${size}" height="${size}" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">${ICONS[name] || ''}</svg>`;
}

const PAGES = [
  ['index.html', 'Home'],
  ['about.html', 'About'],
  ['lifecycle.html', 'Lifecycle'],
  ['incidents.html', 'Incident Types'],
  ['playbooks.html', 'Playbooks'],
  ['roles.html', 'Roles'],
  ['severity.html', 'Severity'],
  ['resources.html', 'Resources'],
  ['contact.html', 'Contact']
];

function buildNav() {
  const current = location.pathname.split('/').pop() || 'index.html';
  const links = PAGES.map(([href, label]) =>
    `<li><a href="${href}" class="${current === href ? 'active' : ''}">${label}</a></li>`).join('');
  const el = document.getElementById('site-header');
  if (!el) return;
  el.innerHTML = `
  <nav class="navbar" aria-label="Main navigation">
    <div class="container nav-inner">
      <a href="index.html" class="brand">${icon('shield', 28)}<span>Firewall<span style="color:var(--red)">101</span><small>Incident Response Framework</small></span></a>
      <button class="menu-btn" id="menuBtn" aria-label="Toggle menu" aria-expanded="false">☰</button>
      <ul class="nav-links" id="navLinks">
        ${links}
        <li><a href="report.html" class="nav-cta ${current === 'report.html' ? 'active' : ''}">Report Incident</a></li>
      </ul>
    </div>
  </nav>`;
  const btn = document.getElementById('menuBtn');
  const list = document.getElementById('navLinks');
  btn.addEventListener('click', () => {
    const open = list.classList.toggle('open');
    btn.setAttribute('aria-expanded', open);
  });
}

function buildFooter() {
  const el = document.getElementById('site-footer');
  if (!el) return;
  el.innerHTML = `
  <footer>
    <div class="container">
      <div class="foot-grid">
        <div>
          <h4>Firewall101</h4>
          <p>Cybersecurity Incident Response Framework for an Educational Institution. An informational college project prototype showing how a college can detect, report, respond to, recover from, and learn from cybersecurity incidents.</p>
        </div>
        <div>
          <h4>Quick links</h4>
          <ul>
            <li><a href="report.html">Report an Incident</a></li>
            <li><a href="playbooks.html">Response Playbooks</a></li>
            <li><a href="resources.html">Resources &amp; Awareness</a></li>
            <li><a href="admin.html">Admin Dashboard (demo)</a></li>
          </ul>
        </div>
        <div>
          <h4>References</h4>
          <ul>
            <li><a href="https://csrc.nist.gov/pubs/sp/800/61/r3/final" target="_blank" rel="noopener">NIST SP 800-61 Rev. 3 – Incident Response</a></li>
            <li><a href="https://csrc.nist.gov/pubs/sp/800/61/r2/final" target="_blank" rel="noopener">NIST SP 800-61 Rev. 2 – Computer Security Incident Handling Guide</a></li>
            <li><a href="https://www.nist.gov/cyberframework" target="_blank" rel="noopener">NIST Cybersecurity Framework</a></li>
          </ul>
        </div>
      </div>
     
    </div>
  </footer>`;
}

/* Report storage: browser localStorage (prototype, no server needed) */
const STATUSES = ['Submitted', 'Under Review', 'Contained', 'Resolved', 'Closed'];
const STORE_KEY = 'ir_reports_v1';
const Store = {
  all() { try { return JSON.parse(localStorage.getItem(STORE_KEY)) || []; } catch (e) { return []; } },
  save(list) { try { localStorage.setItem(STORE_KEY, JSON.stringify(list)); } catch (e) {} },
  add(r) {
    const list = this.all();
    r.id = 'IR-' + new Date().getFullYear() + '-' + String(list.length + 1).padStart(4, '0');
    r.status = 'Submitted';
    r.submittedAt = new Date().toISOString();
    list.unshift(r);
    this.save(list);
    return r;
  },
  setStatus(id, status) {
    const list = this.all();
    const item = list.find(x => x.id === id);
    if (item) { item.status = status; this.save(list); }
  },
  seed() {
    if (this.all().length) return;
    this.save([
      { id: 'IR-2026-0003', submittedAt: '2026-10-26T10:15:00', type: 'Ransomware', severity: 'Critical', name: 'Sample User C', email: 'sample.c@example.edu', dept: 'Admin Office', system: 'Office PC (sample)', description: 'Sample: files renamed and ransom note displayed.', status: 'Under Review', sample: true },
      { id: 'IR-2026-0002', submittedAt: '2026-10-24T14:40:00', type: 'Password / Account Compromise', severity: 'Medium', name: 'Sample User B', email: 'sample.b@example.edu', dept: 'Computer Science', system: 'Student email (sample)', description: 'Sample: unfamiliar login alerts.', status: 'Contained', sample: true },
      { id: 'IR-2026-0001', submittedAt: '2026-10-22T09:05:00', type: 'Phishing Email', severity: 'Low', name: 'Sample User A', email: 'sample.a@example.edu', dept: 'Library', system: 'Email (sample)', description: 'Sample: fake fee-payment email received.', status: 'Resolved', sample: true }
    ]);
  }
};

function statusClass(s) {
  return { 'Submitted': 'st-submitted', 'Under Review': 'st-review', 'Contained': 'st-contained', 'Resolved': 'st-resolved', 'Closed': 'st-closed' }[s] || '';
}
function esc(s) {
  return String(s == null ? '' : s).replace(/[&<>"']/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
}

document.addEventListener('DOMContentLoaded', () => {
  buildNav();
  buildFooter();
  document.querySelectorAll('[data-icon]').forEach(el => {
    el.innerHTML = icon(el.dataset.icon, +(el.dataset.size || 24));
  });
});
