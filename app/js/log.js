/**
 * 统一日志模块 Logger（js/log.js，需最先加载）
 *
 * 功能：
 *  - 结构化日志：毫秒时间戳 + 级别（DEBUG/INFO/WARN/ERROR）+ 模块名 + 消息 + 明细
 *  - 双通道输出：浏览器控制台（按级别着色）+ 内存环形缓冲（默认 2000 条）
 *  - 字节数组/ArrayBuffer 自动转十六进制（便于排查串口报文）
 *  - 全局错误捕获（window error / unhandledrejection）也进入缓冲，
 *    导出日志时与通信日志合并，无需再手动复制控制台
 *  - 页面底部可折叠日志面板：实时滚动显示、级别过滤、清空、导出 .log 文件
 *  - 级别过滤：URL 加 ?log=debug|info|warn|error（优先），
 *    或运行时 Logger.setLevel('warn')（持久化到 localStorage 'acaddia.log.level'）
 *
 * 用法：
 *   Logger.debug('serial', 'RX 原始字节 ×12', bytes);
 *   Logger.info('serial', `发送报文 37 (0x25)`, { hex: Logger.hex(frame) });
 *   Logger.error('serial', '通信超时（3333ms）', { Down, idx, err });
 *   Logger.download('acaddia-日志.log');  // 导出缓冲内容
 */
(function (global) {
    'use strict';

    const LEVELS = { debug: 0, info: 1, warn: 2, error: 3 };
    const LEVEL_TAGS = { debug: 'DEBUG', info: 'INFO ', warn: 'WARN ', error: 'ERROR' };
    const LEVEL_CSS = { debug: 'alp-debug', info: 'alp-info', warn: 'alp-warn', error: 'alp-error' };
    const MAX_BUFFER = 2000;   // 内存环形缓冲条数上限
    const MAX_PANEL = 400;     // 面板显示条数上限
    const MAX_HEX = 512;       // 十六进制明细显示字节数上限
    const LS_KEY = 'acaddia.log.level';

    let currentLevel = LEVELS.debug;
    const buffer = [];
    const hooks = [];

    function pad(n, w) { return String(n).padStart(w, '0'); }

    /** HH:MM:SS.mmm */
    function ts() {
        const d = new Date();
        return pad(d.getHours(), 2) + ':' + pad(d.getMinutes(), 2) + ':' +
            pad(d.getSeconds(), 2) + '.' + pad(d.getMilliseconds(), 3);
    }

    /** 字节数组 → "68 04 04 68 ..."（超长截断并注明总字节数） */
    function hexBytes(bytes, maxLen) {
        const max = maxLen != null ? maxLen : MAX_HEX;
        const arr = Array.from(bytes);
        const shown = arr.slice(0, max);
        const s = shown.map((b) => b.toString(16).padStart(2, '0').toUpperCase()).join(' ');
        return arr.length > max ? s + ' ...（共 ' + arr.length + ' 字节）' : s;
    }

    /** 明细统一格式化：字节数组→hex；Error→name: message；对象→JSON；其余→String */
    function fmtDetail(detail) {
        if (detail === undefined || detail === null) return '';
        if (detail instanceof ArrayBuffer) detail = new Uint8Array(detail);
        if (ArrayBuffer.isView(detail)) return hexBytes(detail);
        if (detail instanceof Error) return detail.name + ': ' + detail.message;
        if (typeof detail === 'object') {
            try { return JSON.stringify(detail); } catch (e) { return String(detail); }
        }
        return String(detail);
    }

    /** 初始级别：URL ?log=xxx 优先，其次 localStorage，默认 debug */
    function resolveLevel() {
        try {
            if (typeof location !== 'undefined' && location.search) {
                const m = /[?&]log=(debug|info|warn|error)/i.exec(location.search);
                if (m) return m[1].toLowerCase();
            }
            if (typeof localStorage !== 'undefined') {
                const v = localStorage.getItem(LS_KEY);
                if (v && LEVELS[v] != null) return v;
            }
        } catch (e) { /* 隐私模式等场景忽略 */ }
        return 'debug';
    }

    function write(level, module, msg, detail) {
        if (LEVELS[level] < currentLevel) return;
        const t = ts();
        const det = fmtDetail(detail);
        const line = '[' + t + '] [' + LEVEL_TAGS[level] + '] [' + module + '] ' + msg +
            (det ? ' ' + det : '');
        const entry = { t, level, module, msg, detail: det };
        buffer.push(entry);
        if (buffer.length > MAX_BUFFER) buffer.splice(0, buffer.length - MAX_BUFFER);
        // 控制台同步输出（debug 走 log，便于直接查看；其他按级别对应方法）
        try {
            const fn = (level === 'debug') ? console.log
                : (level === 'info') ? console.info
                : (level === 'warn') ? console.warn : console.error;
            if (fn) fn(line);
        } catch (e) { /* 无 console 环境（node 测试等）忽略 */ }
        hooks.slice().forEach((fn) => {
            try { fn(entry); } catch (e) { /* 钩子异常不影响日志主流程 */ }
        });
    }

    const Logger = {
        LEVELS,

        debug(module, msg, detail) { write('debug', module, msg, detail); },
        info(module, msg, detail) { write('info', module, msg, detail); },
        warn(module, msg, detail) { write('warn', module, msg, detail); },
        error(module, msg, detail) { write('error', module, msg, detail); },

        /** 字节数组 → 十六进制字符串 */
        hex(bytes, maxLen) { return hexBytes(bytes, maxLen); },

        /** 获取当前级别名 */
        getLevel() {
            return Object.keys(LEVELS).find((k) => LEVELS[k] === currentLevel) || 'debug';
        },

        /** 设置级别（debug|info|warn|error 或对应数字），并持久化 */
        setLevel(level) {
            const name = typeof level === 'number' ? Object.keys(LEVELS).find((k) => LEVELS[k] === level) : level;
            if (!name || LEVELS[name] == null) return false;
            currentLevel = LEVELS[name];
            try { if (typeof localStorage !== 'undefined') localStorage.setItem(LS_KEY, name); } catch (e) { /* ignore */ }
            return true;
        },

        /** 缓冲副本（最近 MAX_BUFFER 条） */
        getLogs() { return buffer.slice(); },

        /** 清空缓冲（面板由钩子同步） */
        clear() { buffer.length = 0; },

        /** 导出缓冲为 .log 文件（无 DOM 环境退化为控制台打印） */
        download(filename) {
            const text = buffer.map((e) =>
                '[' + e.t + '] [' + LEVEL_TAGS[e.level] + '] [' + e.module + '] ' + e.msg +
                (e.detail ? ' ' + e.detail : '')).join('\n') + '\n';
            if (typeof document === 'undefined' || typeof Blob === 'undefined') {
                try { console.log(text); } catch (e) { /* ignore */ }
                return;
            }
            const blob = new Blob([text], { type: 'text/plain;charset=utf-8' });
            const a = document.createElement('a');
            a.href = URL.createObjectURL(blob);
            const d = new Date();
            const stamp = d.getFullYear() + pad(d.getMonth() + 1, 2) + pad(d.getDate(), 2) +
                '-' + pad(d.getHours(), 2) + pad(d.getMinutes(), 2) + pad(d.getSeconds(), 2);
            a.download = filename || ('acaddia-日志-' + stamp + '.log');
            a.click();
            URL.revokeObjectURL(a.href);
        },

        /** 订阅日志条目（面板等使用） */
        onLog(fn) { hooks.push(fn); },

        /**
         * 全局错误捕获：未捕获异常 / Promise 拒绝都进入日志缓冲，
         * 与通信日志一起导出，便于排查（不拦截、不改变默认行为）
         */
        installErrorHandlers() {
            if (typeof window === 'undefined') return;
            window.addEventListener('error', (ev) => {
                try {
                    Logger.error('global', '未捕获异常',
                        (ev.message || String(ev.error)) + ' @ ' + ev.filename + ':' + ev.lineno + ':' + ev.colno);
                } catch (e) { /* ignore */ }
            });
            window.addEventListener('unhandledrejection', (ev) => {
                try { Logger.error('global', '未处理的 Promise 拒绝', ev.reason); } catch (e) { /* ignore */ }
            });
        },

        /**
         * 挂载页面底部日志面板（可折叠；级别下拉同步 Logger 级别）
         */
        mountPanel() {
            if (typeof document === 'undefined' || document.getElementById('acaddia-log-panel')) return;

            const panel = document.createElement('div');
            panel.id = 'acaddia-log-panel';
            panel.className = 'alp-panel';
            panel.innerHTML =
                '<div class="alp-header">' +
                '  <span class="alp-title">通信日志</span>' +
                '  <select class="alp-level" title="日志级别过滤">' +
                '    <option value="debug">DEBUG</option>' +
                '    <option value="info">INFO</option>' +
                '    <option value="warn">WARN</option>' +
                '    <option value="error">ERROR</option>' +
                '  </select>' +
                '  <button type="button" class="alp-btn" data-act="clear" title="清空日志">清空</button>' +
                '  <button type="button" class="alp-btn" data-act="export" title="导出 .log 文件">导出</button>' +
                '  <button type="button" class="alp-btn" data-act="toggle" title="收起/展开">收起</button>' +
                '</div>' +
                '<div class="alp-body"></div>';
            document.body.appendChild(panel);

            const body = panel.querySelector('.alp-body');
            const sel = panel.querySelector('.alp-level');
            sel.value = Logger.getLevel();
            sel.addEventListener('change', () => Logger.setLevel(sel.value));

            // 用当前缓冲回放历史，再订阅新条目
            Logger.getLogs().forEach((e) => appendLine(body, e));
            Logger.onLog((e) => appendLine(body, e));

            panel.querySelectorAll('.alp-btn').forEach((btn) => {
                btn.addEventListener('click', () => {
                    const act = btn.getAttribute('data-act');
                    if (act === 'clear') {
                        Logger.clear();
                        body.textContent = '';
                    } else if (act === 'export') {
                        Logger.download();
                    } else if (act === 'toggle') {
                        panel.classList.toggle('alp-collapsed');
                        btn.textContent = panel.classList.contains('alp-collapsed') ? '展开' : '收起';
                    }
                });
            });

            function appendLine(el, e) {
                const div = document.createElement('div');
                div.className = 'alp-line ' + LEVEL_CSS[e.level];
                div.textContent = '[' + e.t + '] [' + LEVEL_TAGS[e.level] + '] [' + e.module + '] ' +
                    e.msg + (e.detail ? ' ' + e.detail : '');
                el.appendChild(div);
                while (el.childNodes.length > MAX_PANEL) el.removeChild(el.firstChild);
                el.scrollTop = el.scrollHeight;
            }
        },
    };

    currentLevel = LEVELS[resolveLevel()];
    global.Logger = Logger;
})(typeof window !== 'undefined' ? window : globalThis);
