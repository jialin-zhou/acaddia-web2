/**
 * 日志分析工具：node test/analyze-log.js <日志文件>
 * 解析导出日志中的 36 号帧（AD计算值），按每次"点击 获取信息"分组，
 * 输出 DC1~DC8 与 Line1/Line2 各 14 项原始值及物理值（与 Qt updateData 公式一致）。
 */
const fs = require('fs');

const file = process.argv[2];
if (!file) { console.error('用法: node test/analyze-log.js <日志文件>'); process.exit(1); }
const text = fs.readFileSync(file, 'utf8');
const lines = text.split('\n');

const fetches = [];  // { startLine, frames: [{t, nr, dataBlock, raw}] }
let cur = null;

for (let i = 0; i < lines.length; i++) {
    const ln = lines[i];
    if (ln.includes('[app] 点击 获取信息')) {
        cur = { startLine: i + 1, t: ln.slice(1, 13), frames: [] };
        fetches.push(cur);
        continue;
    }
    const m = ln.match(/^\[([^\]]+)\] \[INFO \] \[serial\] RX 数据帧 Nr=(\d+) \(0x[0-9a-f]+\) \{"hex":"([0-9A-F ]+)"/);
    if (m && cur) {
        const hex = m[3].split(' ').filter(Boolean).map((h) => parseInt(h, 16));
        const dataBlock = hex.slice(4, 4 + hex[1]); // 68 L L 68 之后
        cur.frames.push({ t: m[1], nr: parseInt(m[2], 10), dataBlock, raw: hex });
    }
    // 获取结束标记当前分组
    if (ln.includes('主界面获取结束') && cur) {
        cur.endLine = i + 1;
        cur = null;
    }
}

const ITEMS = ['Ua', 'Ub', 'Uc', 'Uab', 'Ubc', 'Uca', 'Ia', 'Ib', 'Ic', 'P', 'Q', 'S', 'f', 'Cos phi'];
const DIM_PUBLIC = 16384;

fetches.forEach((f, fi) => {
    console.log(`\n========== 第 ${fi + 1} 次获取（点击于 ${f.t}，日志行 ${f.startLine}${f.endLine ? '-' + f.endLine : ''}） ==========`);
    f.frames.forEach((fr) => {
        console.log(`\n--- RX Nr=${fr.nr} @ ${fr.t}（dataBlock ${fr.dataBlock.length} 字节） ---`);
        if (fr.nr !== 36) { console.log('    hex:', fr.raw.map((b) => b.toString(16).padStart(2, '0').toUpperCase()).join(' ')); return; }
        const vd = fr.dataBlock;
        // DC1~DC8（Qt: vd[5+2j]<<8 | vd[4+2j]，大端；select=1 → ×10 mA）
        for (let j = 0; j < 8; j++) {
            const raw = (vd[5 + 2 * j] << 8) | vd[4 + 2 * j];
            const norm = raw / DIM_PUBLIC;
            const phy = norm * 10; // DC_V_I_select 连接后全 1 → mA
            console.log(`  DC${j + 1}: raw=0x${raw.toString(16).padStart(4, '0')} (${raw})  = ${phy.toFixed(4)} mA`);
        }
        // Line1 / Line2（Qt 大端）
        [['Line1', 20, 16384], ['Line2', 48, DIM_PUBLIC]].forEach(([name, base, div]) => {
            const parts = [];
            for (let j = 0; j < 14; j++) {
                const raw = (vd[base + 1 + 2 * j] << 8) | vd[base + 2 * j];
                parts.push(`${ITEMS[j]}=0x${raw.toString(16).padStart(4, '0')}(${raw})`);
            }
            console.log(`  ${name}: ${parts.join(' ')}`);
        });
    });
});
if (!fetches.length) console.log('未找到获取记录');
