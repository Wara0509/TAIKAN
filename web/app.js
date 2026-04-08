const ENV_OPTIONS = [
  { key: "use_bridge", label: "橋を通る" },
  { key: "use_windy_place", label: "風が強い場所" },
  { key: "use_atrium", label: "吹き抜け" },
  { key: "use_high_place", label: "高所" },
  { key: "use_shade", label: "日陰が多い" },
  { key: "use_sunny", label: "日向が多い" },
  { key: "use_indoor", label: "室内が多い" }
];

const MOVE_OPTIONS = [
  { key: "move_walk_lots", label: "徒歩多い" },
  { key: "move_bicycle", label: "自転車" },
  { key: "move_still", label: "ほぼ動かない" },
  { key: "move_normal", label: "普通" }
];

const TIME_OPTIONS = [
  { key: "time_cold_morning", label: "朝寒い" },
  { key: "time_hot_noon", label: "昼暑い" },
  { key: "time_cold_evening", label: "夕方寒い" }
];

const state = {
  env: {},
  move: "move_normal",
  time: {}
};

const $ = (id) => document.getElementById(id);

function initUI() {
  const envRoot = $("envChips");
  ENV_OPTIONS.forEach((o) => {
    const b = document.createElement("button");
    b.className = "chip";
    b.textContent = o.label;
    b.onclick = () => {
      state.env[o.key] = !state.env[o.key];
      b.classList.toggle("selected", !!state.env[o.key]);
    };
    envRoot.appendChild(b);
  });

  const moveRoot = $("moveRadios");
  MOVE_OPTIONS.forEach((o) => {
    const b = document.createElement("button");
    b.className = "chip";
    b.textContent = o.label;
    b.onclick = () => {
      state.move = o.key;
      [...moveRoot.children].forEach((n) => n.classList.remove("selected"));
      b.classList.add("selected");
    };
    if (o.key === state.move) b.classList.add("selected");
    moveRoot.appendChild(b);
  });

  const timeRoot = $("timeChips");
  TIME_OPTIONS.forEach((o) => {
    const b = document.createElement("button");
    b.className = "chip";
    b.textContent = o.label;
    b.onclick = () => {
      state.time[o.key] = !state.time[o.key];
      b.classList.toggle("selected", !!state.time[o.key]);
    };
    timeRoot.appendChild(b);
  });

  document.querySelectorAll(".next").forEach((b) => b.addEventListener("click", () => gotoStep(Number(b.dataset.next))));
  document.querySelectorAll(".prev").forEach((b) => b.addEventListener("click", () => gotoStep(Number(b.dataset.prev))));
  $("quickRun").addEventListener("click", runQuick);
  $("preciseRun").addEventListener("click", runPrecise);
}

function gotoStep(step) {
  document.querySelectorAll(".step").forEach((el) => el.classList.toggle("active", Number(el.dataset.step) === step));
  [1, 2, 3].forEach((s) => $("step" + s).classList.toggle("active", s === step));
}

function getWeatherInput() {
  return {
    location: $("location").value || "Tokyo",
    date: $("date").value || "2026-04-08",
    weather: $("weather").value || "Cloudy",
    temp: Number($("temp").value || 18),
    wind: Number($("wind").value || 0)
  };
}

function behaviorFromState() {
  return {
    ...Object.fromEntries(ENV_OPTIONS.map((o) => [o.key, !!state.env[o.key]])),
    move_walk_lots: state.move === "move_walk_lots",
    move_bicycle: state.move === "move_bicycle",
    move_still: state.move === "move_still",
    time_cold_morning: !!state.time.time_cold_morning,
    time_hot_noon: !!state.time.time_hot_noon,
    time_cold_evening: !!state.time.time_cold_evening
  };
}

function windCorrection(wind) {
  if (wind >= 8) return 4;
  if (wind >= 5) return 2;
  if (wind >= 3) return 1;
  return 0;
}

function placeCorrection(b) {
  let c = 0;
  if (b.use_bridge) c -= 1.5;
  if (b.use_high_place) c -= 1.0;
  if (b.use_atrium) c -= 1.0;
  if (b.use_shade) c -= 0.5;
  if (b.use_sunny) c += 1.0;
  return c;
}

function activityCorrection(b) {
  if (b.move_walk_lots || b.move_bicycle) return 1.5;
  if (!b.move_still) return 0.5;
  return 0;
}

function calculateFeelsLike(temp, wind, b) {
  return temp - windCorrection(wind) + placeCorrection(b) + activityCorrection(b);
}

function calculateDistribution(temp, wind, b) {
  const outdoor = calculateFeelsLike(temp, wind, b)
    + (b.time_hot_noon ? 1.5 : 0)
    - (b.time_cold_morning ? 1.0 : 0)
    - (b.time_cold_evening ? 1.0 : 0);

  const bridgeB = { ...b, use_bridge: true };
  const bridge = calculateFeelsLike(temp, wind, bridgeB)
    - (b.time_cold_morning ? 1.0 : 0)
    - (b.time_cold_evening ? 1.0 : 0);

  const indoor = outdoor + 3.0 + (b.use_indoor ? 0.5 : 0) + (b.time_hot_noon ? 1.0 : 0);
  const min = Math.min(bridge, outdoor, indoor);
  const max = Math.max(bridge, outdoor, indoor);

  return { bridge, outdoor, indoor, min, max, diff: max - min };
}

function analyzeStrategy(dist, wind, b) {
  const main = dist.diff < 3 ? "安定" : dist.diff >= 6 ? "調整前提" : "軽い調整";
  const sub = [];
  if (wind >= 5 || b.use_windy_place || b.use_bridge) sub.push("防風対策");
  if (dist.min < 13) sub.push("寒さリスク");
  if (dist.max > 24) sub.push("暑さリスク");
  if (b.move_walk_lots || b.move_bicycle) sub.push("軽装推奨");
  if (sub.length === 0) sub.push("標準対応");

  const strategy = `${main} + ${sub.join(" + ")}`;
  const comment = wind >= 8
    ? "風が強く体感が下がるため、防風を優先。"
    : dist.diff >= 6
      ? "場所と時間で体感差が大きく、脱ぎ着前提。"
      : "体感差は中程度。1枚追加で微調整。";

  return {
    strategy,
    comment,
    guidance: "羽織りは必須 / 脱ぎ着前提 / 防風素材を含める",
    ng: "1枚で完結する服",
    outfit: "例: 吸湿インナー + 長袖 + 軽量防風シェル"
  };
}

function renderFull(weather, dist, decision) {
  return `【戦略】\n${decision.strategy}\n
【体感温度の分布】\n橋: ${dist.bridge.toFixed(1)}℃\n外: ${dist.outdoor.toFixed(1)}℃\n室内: ${dist.indoor.toFixed(1)}℃\n
【状態】\n温度差: ${dist.diff.toFixed(1)}℃\n最低体感: ${dist.min.toFixed(1)}℃\n最高体感: ${dist.max.toFixed(1)}℃\n
【行動指針】\n${decision.guidance}\n
【NG例】\n${decision.ng}\n
【服装イメージ】\n${decision.outfit}\n
補足: ${decision.comment}`;
}

function runQuick() {
  const w = getWeatherInput();
  const b = behaviorFromState();
  const dist = calculateDistribution(w.temp, w.wind, b);
  const decision = analyzeStrategy(dist, w.wind, b);
  const feels = calculateFeelsLike(w.temp, w.wind, b);

  $("quickOutput").textContent = `地域: ${w.location}\n日付: ${w.date}\n天気: ${w.weather}\n気温: ${w.temp.toFixed(1)}℃\n体感温度: ${feels.toFixed(1)}℃\n体感差: ${(feels - w.temp).toFixed(1)}℃\n\n戦略: ${decision.strategy}\n補足: ${decision.comment}`;
}

function runPrecise() {
  const w = getWeatherInput();
  const b = behaviorFromState();
  const dist = calculateDistribution(w.temp, w.wind, b);
  const decision = analyzeStrategy(dist, w.wind, b);
  $("result").classList.remove("muted");
  $("result").textContent = renderFull(w, dist, decision);
}

initUI();
