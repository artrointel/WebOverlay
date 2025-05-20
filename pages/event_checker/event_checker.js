const EVENTS = [
  { index: 1, label: "지옥길", icon: "https://icons.iconarchive.com/icons/google/noto-emoji-travel-places/256/42697-fire-icon.png" },
  { index: 2, label: "케이론", icon: "https://icons.iconarchive.com/icons/seanau/user/256/Thief-icon.png" },
  { index: 3, label: "다크사이트", icon: "https://icons.iconarchive.com/icons/papirus-team/papirus-apps/256/amnesia-the-dark-descent-icon.png" },
  { index: 4, label: "타락한낙원", icon: "https://icons.iconarchive.com/icons/mathijssen/tuxlets/256/Devil-Tux-icon.png" },
  { index: 5, label: "디스코", icon: "https://icons.iconarchive.com/icons/tanitakawkaw/simple-cute/256/music-icon.png" }
];

function saveGlobalSettingsToCookie() {
  const volume = document.getElementById('tts-volume-range').value;
  document.cookie = `ttsVolume=${volume}; path=/; max-age=31536000`;
}

function loadGlobalSettingsFromCookie() {
  const match = document.cookie.match(/(?:^|; )ttsVolume=([^;]*)/);
  const volume = match ? parseInt(match[1]) : 50;
  document.getElementById('tts-volume-range').value = volume;
  window.__ttsVolume = volume / 100;
}
    
const timers = new Map();

function speak(text) {
  const utterance = new SpeechSynthesisUtterance(text);
  utterance.lang = "ko-KR";
  utterance.pitch = 1.5;
  utterance.rate = 1.3;
  utterance.volume = window.__ttsVolume ?? 0.5;
  speechSynthesis.speak(utterance);
}

function resetTimer(el, isReset = false) {

  const timerEl = el.previousElementSibling;
  const eventLabelEl = el.parentElement.querySelector('.event-label');
  const progressBar = el.parentElement.nextElementSibling;
  const progressEl = progressBar.querySelector('.progress');
  const characterName = el.closest('.character-box').querySelector('.character-name').value || '캐릭터';
  const eventLabel = eventLabelEl.textContent;

  const labelText = el.closest('.event-row')?.querySelector('.event-label')?.textContent?.trim();
  const eventInfo = EVENTS.find(ev => ev.label === labelText);
  const index = eventInfo?.index;

  const interval = timers.get(timerEl);
  if (interval) clearInterval(interval);

  eventLabelEl.style.opacity = '1';
  timerEl.textContent = '00:00:00';
  progressEl.style.width = '0%';
  progressBar.style.display = 'none';
  el.textContent = 'Ctrl+' + index;

  if (!isReset) {
    const alarmToggle = el.closest('.character-box').querySelector('.alarm-check');
    if (alarmToggle.checked) {
      speak(`${characterName}, ${eventLabel} 이벤트 고고!`);
    }
  }
}

function startTimer(el) {
  const timerEl = el.previousElementSibling;
  const eventLabelEl = el.parentElement.querySelector('.event-label');
  const progressBar = el.parentElement.nextElementSibling;
  const progressEl = progressBar.querySelector('.progress');

  if (el.textContent === 'Reset') {
    resetTimer(el, true);
    return;
  }

  eventLabelEl.style.opacity = '0.5';
  progressBar.style.display = 'block';
  el.textContent = 'Reset';

  const startTimestamp = Date.now();
  const endTimestamp = startTimestamp + 60 * 60 * 1000;

  const interval = setInterval(() => {
    const now = Date.now();
    let remaining = Math.max(0, Math.floor((endTimestamp - now) / 1000));

    const h = String(Math.floor(remaining / 3600)).padStart(2, '0');
    const m = String(Math.floor((remaining % 3600) / 60)).padStart(2, '0');
    const s = String(remaining % 60).padStart(2, '0');
    timerEl.textContent = `${h}:${m}:${s}`;

    progressEl.style.width = `${((3600 - remaining) / 3600) * 100}%`;

    if (remaining <= 0) {
      clearInterval(interval);
      resetTimer(el);
    }
  }, 1000);

  timers.set(timerEl, interval);
}

function addCharacterBox(name = '') {
  const box = document.createElement('div');
  box.className = 'character-box';
  box.innerHTML = `
    <input class="character-name" type="text" placeholder="캐릭터명" value="${name}" onchange="saveCharacterNamesToCookie()" />
    ${EVENTS.map(ev => `
      <div class="event-row">
        <div class="event-main">
          <img src="${ev.icon}" alt="아이콘" class="event-icon" />
          <div class="event-label">${ev.label}</div>
          <div class="timer">00:00:00</div>
          <button class="complete-btn">Ctrl+${ev.index}</button>
        </div>
        <div class="progress-bar" style="display: none;"><div class="progress"></div></div>
      </div>
    `).join('')}
    <div class="alarm-toggle">
      <span><input type="checkbox" class="alarm-check" checked /> <label>음성 알림 켜기</label></span>
      <button class="delete-btn" onclick="deleteCharacterBox(this)">삭제</button>
    </div>
  `;
  box.querySelectorAll('.complete-btn').forEach(btn => {
    btn.addEventListener('click', () => startTimer(btn));
  });
  document.getElementById('container').appendChild(box);
  saveCharacterNamesToCookie();
}

function deleteCharacterBox(btn) {
  const box = btn.closest('.character-box');
  box.remove();
  saveCharacterNamesToCookie();
}

function toggleTheme() {
  const isDark = document.body.classList.toggle('dark');
  document.cookie = `theme=${isDark ? 'dark' : 'light'}; path=/; max-age=31536000`;
}

function saveCharacterNamesToCookie() {
  const names = Array.from(document.querySelectorAll('.character-name')).map(input => input.value);
  document.cookie = `characterNames=${encodeURIComponent(JSON.stringify(names))}; path=/; max-age=31536000`;
}

function loadCharacterNamesFromCookie() {
  const match = document.cookie.match(/(?:^|; )characterNames=([^;]*)/);
  var characterSize = 0
  if (match) {
    try {
      const names = JSON.parse(decodeURIComponent(match[1]));
      if (Array.isArray(names)) {
        names.forEach(name => {
          addCharacterBox(name);
          characterSize++;
        });
        return;
      }
    } catch (e) {
      console.error('쿠키 파싱 에러:', e);
    }
  }
  if (characterSize == 0) {
    addCharacterBox();
  }
}

window.addEventListener("keydown", function (e) {
  if (e.ctrlKey) {
    const index = parseInt(e.key);
    const targetEvent = EVENTS.find(ev => ev.index === index);
    if (!targetEvent) return;

    const allBoxes = document.querySelectorAll(".character-box");
    for (const box of allBoxes) {
      const rows = box.querySelectorAll(".event-row");
      for (const row of rows) {
        const main = row.querySelector(".event-main");
        if (!main) continue;

        const label = main.querySelector(".event-label");
        const button = main.querySelector(".complete-btn");

        if (label && label.textContent.trim() === targetEvent.label && button) {
          button.click();
          return;
        }
      }
    }
  }
});
document.body.classList.add('dark');
(function loadThemeFromCookie() {
  const match = document.cookie.match(/(?:^|; )theme=([^;]*)/);
  if (match && match[1] === 'dark') {
    document.body.classList.add('dark');
  }
})();

window.addEventListener('DOMContentLoaded', () => {
  loadGlobalSettingsFromCookie();
  loadCharacterNamesFromCookie();
});
