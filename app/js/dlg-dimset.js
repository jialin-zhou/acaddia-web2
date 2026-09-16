/**
 * 标幺设置对话框（DLg_DIMset）——纯本地设置，不走串口
 * 单选切换立即写回全局（Qt: 单选槽立即设置 pParent->DIM_public / DIM_Line_Vot_style）
 */
const DlgDim = {
    open() {
        // 按当前全局状态勾选
        document.querySelectorAll('input[name="dim-global"]').forEach((r) => {
            r.checked = (parseInt(r.value, 10) === Qt.DIM_public);
        });
        document.querySelectorAll('input[name="dim-volt"]').forEach((r) => {
            r.checked = (parseInt(r.value, 10) === Qt.DIM_Line_Vot_style);
        });
        openDlg('dlg-dim');
    },

    close() {
        closeDlg('dlg-dim');
    },

    restoreDefault() {
        // Qt onButtonOKClicked（"默认设置"）：DIM_public=16384, DIM_Line_Vot_style=0
        Qt.DIM_public = 16384;
        Qt.DIM_Line_Vot_style = 0;
        document.querySelectorAll('input[name="dim-global"]').forEach((r) => {
            r.checked = (parseInt(r.value, 10) === 16384);
        });
        document.querySelectorAll('input[name="dim-volt"]').forEach((r) => {
            r.checked = (parseInt(r.value, 10) === 0);
        });
        // 主界面立即刷新（标幺影响换算基数）
        Bus.emit('dim-changed');
    },
};

(function () {
    document.querySelectorAll('input[name="dim-global"]').forEach((r) => {
        r.addEventListener('change', () => {
            if (r.checked) {
                Qt.DIM_public = parseInt(r.value, 10);
                Bus.emit('dim-changed');
            }
        });
    });
    document.querySelectorAll('input[name="dim-volt"]').forEach((r) => {
        r.addEventListener('change', () => {
            if (r.checked) {
                Qt.DIM_Line_Vot_style = parseInt(r.value, 10);
                Bus.emit('dim-changed');
            }
        });
    });
    $('dim-default').addEventListener('click', () => DlgDim.restoreDefault());
    // Qt 命名相反："确定"按钮对应 onButtonCancelClicked，仅关闭窗口
    $('dim-ok').addEventListener('click', () => DlgDim.close());
})();
