/**
 * 一键运行全部测试：node test/run-all.js
 */
const { execSync } = require('child_process');
const path = require('path');

const suites = ['protocol-test.js', 'state-machine-test.js', 'data-flow.test.js'];
let failed = 0;
for (const s of suites) {
    console.log(`\n${'='.repeat(70)}\n>>> 运行 ${s}\n${'='.repeat(70)}`);
    try {
        execSync(`"${process.execPath}" "${path.join(__dirname, s)}"`, { stdio: 'inherit' });
    } catch (e) {
        failed++;
    }
}
console.log(`\n${'='.repeat(70)}`);
console.log(failed === 0 ? '全部测试套件通过 ✓' : `${failed} 个套件失败 ✗`);
process.exit(failed === 0 ? 0 : 1);
