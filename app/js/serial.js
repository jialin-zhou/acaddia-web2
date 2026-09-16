/**
 * Web Serial 串口管理 + Qt 通信状态机
 *
 * Web Serial API 用法参考 Vue 版（acaddia-web/src/App.vue）：
 *   port.writable.getWriter() / port.readable.getReader() 循环读取、Unpacker 流式解帧。
 *
 * 交互状态机严格按 Qt 版 new1Dlg.cpp 移植：
 *   在线连接(Down=1001)：发35→收34→发33→收32→发37→收36→发47→收48→完成
 *   主界面获取(Down=1005)：发37→收36→发47→收48→发47→收48→发49→收50→完成
 *   收到数据帧回 0xA2；连接期收到 E5/未知帧计错误并重发35；错误≥5次报通信错误
 *   （Qt m_timer12=3333ms 超时；Qt 原实现只对第一级(35)起超时，
 *    这里对每级都套用 3333ms 超时以等价表达 Qt 意图，字节交互不变）
 *
 * 与 Qt 的等价差异（Qt 自身无明确行为处，按合理方式处理）：
 *   - 获取流程收到裸 E5：Qt 对 Down==1005 的 E5 不做任何处理，状态机停在等待 50 号帧上，
 *     仅靠 OnButtonZjmFetch 的 QTimer::singleShot(999) 刷新界面（数据照常显示）。
 *     本实现等价处理：设备已确认命令且无数据帧可回 → 置 RX_Msg_End 并结束获取、
 *     刷新已收到的数据，避免无休止等待。
 *   - 获取流程超时：Qt handleTimer12 只弹错误不重试；本实现按"每级超时"设计重发 37，
 *     但每次重试计入 ErrorCounter（Qt 判据 ErrorCounter>4 报通信错误），
 *     避免设备不响应时无限循环。
 */

const SerialManager = {
    port: null,
    writer: null,
    reader: null,
    keepReading: false,
    unpacker: new Unpacker(),

    // 当前待决命令（对话框操作）
    pendingCommand: null,
    // 各阶段超时定时器
    stageTimer: null,

    COMM_TIMEOUT: 3333, // Qt m_timer12

    // ---------------- 连接管理 ----------------

    async requestPort() {
        // 客户端授权，参考 Vue 版
        Logger.debug('serial', '请求用户选择串口（navigator.serial.requestPort）');
        const port = await navigator.serial.requestPort();
        return port;
    },

    /**
     * 打开串口并启动读取循环（参数与 Qt Dlg_ComSet 一致）
     */
    async connect(port, settings) {
        if (this.port) await this.disconnect();

        this.port = port;
        await port.open({
            baudRate: settings.baudRate,
            dataBits: settings.dataBits,
            stopBits: settings.stopBits,
            parity: settings.parity,
            flowControl: settings.flowControl || 'none',
            bufferSize: settings.bufferSize || 255,
        });

        this.writer = port.writable.getWriter();
        this.reader = port.readable.getReader();
        this.keepReading = true;
        this.unpacker.reset();
        Qt.serial.connected = true;
        Qt.serial.settings = settings;
        const info = port.getInfo && port.getInfo();
        Qt.serial.portName = info && info.usbVendorId ? `VID:${info.usbVendorId.toString(16)}, PID:${info.usbProductId.toString(16)}` : '已连接';
        Logger.info('serial', '串口已打开', {
            port: Qt.serial.portName,
            baudRate: settings.baudRate, parity: settings.parity,
            dataBits: settings.dataBits, stopBits: settings.stopBits,
        });
        Bus.emit('serial-connected', Qt.serial);
        Bus.emit('status', '串口已打开，正在与装置通信……');
        this.readLoop();
    },

    async disconnect() {
        this.keepReading = false;
        this.clearStageTimer();
        this.rejectPending('串口已断开');
        if (this.writer) {
            try { await this.writer.abort('disconnect'); } catch (e) { /* ignore */ }
            try { this.writer.releaseLock(); } catch (e) { /* ignore */ }
            this.writer = null;
        }
        if (this.reader) {
            try { await this.reader.cancel('disconnect'); } catch (e) { /* ignore */ }
            try { this.reader.releaseLock(); } catch (e) { /* ignore */ }
            this.reader = null;
        }
        if (this.port) {
            try { await this.port.close(); } catch (e) { /* ignore */ }
            this.port = null;
        }
        Qt.serial.connected = false;
        Logger.info('serial', '串口已关闭');
        Bus.emit('serial-disconnected');
    },

    // ---------------- 收发 ----------------

    async write(data) {
        if (!this.writer || !this.port) {
            throw new Error('串口未打开');
        }
        await this.writer.write(data);
    },

    /**
     * 读取循环：按 Qt 方式以 100ms 静默窗口批量处理
     * （Qt: handleReadyRead 收字节入 RXDAT → 100ms 无新数据 → handleTimer11 解析整块缓冲）
     * 这样设备连发的 "E5 + 68帧" 在同一窗口内按数据帧整体处理（与 Qt TeleUnpack 一致），
     * 裸 E5（独立窗口）才按短应答处理。
     */
    async readLoop() {
        let batch = [];
        let silenceTimer = null;
        while (this.port && this.reader && this.keepReading) {
            try {
                const { value, done } = await this.reader.read();
                if (done) break;
                if (value && value.length > 0) {
                    Logger.debug('serial', `RX 原始字节 ×${value.length}`, value);
                    batch.push(...value);
                    clearTimeout(silenceTimer);
                    silenceTimer = setTimeout(() => {
                        const bytes = batch;
                        batch = [];
                        this.processBatch(bytes);
                    }, 100); // Qt m_timer11
                }
            } catch (e) {
                if (e.name === 'AbortError' || (e.message || '').includes('cancel')) {
                    Logger.debug('serial', '读取已取消（AbortError）');
                } else {
                    Logger.error('serial', '读取错误', e);
                    await showMessageBox('错误', '设备已断开连接');
                    await this.disconnect();
                }
                break;
            }
        }
        clearTimeout(silenceTimer);
        if (batch.length) this.processBatch(batch);
        Logger.debug('serial', '读取循环退出');
    },

    /**
     * 处理一个 100ms 静默窗口内的数据（对应 handleTimer11 的 TeleUnpack）
     * E5 后紧跟数据帧 → 帧前缀，按数据帧处理（Qt 中整帧识别，ACK1 不置位）
     */
    async processBatch(bytes) {
        this.unpacker.reset();
        this.unpacker.addData(new Uint8Array(bytes));
        const frames = this.unpacker.unpack();
        for (let i = 0; i < frames.length; i++) {
            const frame = frames[i];
            Bus.emit('frame', frame);
            if (frame.type === 'data') {
                await this.onDataFrame(frame);
            } else if (frame.type === 'ack_e5') {
                // E5 紧跟着数据帧 → 帧前缀（设备发送 "E5 68 L L 68 ... 16"）
                if (i + 1 < frames.length && frames[i + 1].type === 'data') {
                    Logger.debug('serial', 'E5 为数据帧前缀（同窗口），按数据帧处理');
                    continue;
                }
                this.onE5();
            } else if (frame.type === 'ack_a2') {
                this.onA2();
            } else {
                // junk：无法解析的字节
                Logger.warn('serial', '收到无法解析的字节（junk）', frame.raw);
            }
        }
    },

    // ---------------- 帧分发（对应 new1Dlg::handleTimer11） ----------------

    /**
     * 收到数据帧：
     * 1) 已知报文号 → 存入注册表 + 回 0xA2（Qt: 各分发分支均回 A2）
     * 2) 未知报文号 → 不回 A2，ErrorCounter++，Connect_Index=0（Qt 原逻辑）
     * 3) 驱动连接/获取状态机
     * 4) 解析待决对话框命令
     */
    async onDataFrame(frame) {
        const nr = frame.telegramNr;
        Logger.info('serial', `RX 数据帧 Nr=${nr} (0x${nr.toString(16)})`, {
            hex: Logger.hex(frame.raw),
            len: frame.raw.length,
            state: stateTag(),
        });

        const known = Qt.saveResponse(frame);

        if (known) {
            // 回主站应答 A2（Qt: master -> slave）
            try { await this.write(packAck()); }
            catch (e) {
                Logger.error('serial', '回 A2 失败', e);
            }
        } else {
            // Qt: else { ErrorCounter++; Connect_Index = 0; }
            Qt.ErrorCounter++;
            Qt.Connect_Index = 0;
            Logger.warn('serial', `未知报文号 Nr=${nr}，ErrorCounter=${Qt.ErrorCounter}，Connect_Index=0`);
        }

        // 连接/获取状态机
        if (Qt.connecting && Qt.Down === 1001) {
            this.stepConnectChain(frame, known);
            return;
        }
        if (Qt.fetching && Qt.Down === 1005) {
            this.stepFetchChain(frame, known);
            return;
        }

        // 对话框待决命令
        const cmd = this.pendingCommand;
        if (cmd) {
            const expect = cmd.expectNrs || (cmd.expectNr != null ? [cmd.expectNr] : null);
            if (expect && expect.includes(nr)) {
                this.resolvePending(frame);
            }
            // 其余意外帧不解决命令，等待超时（Qt 中对话框只按响应号解析）
        }
    },

    /**
     * 收到 E5 短应答（独立窗口，无后续帧）：
     * - 待决命令期待 E5（Apply 类）→ 成功
     * - 连接期(Down=1001) → ErrorCounter++，Connect_Index=0，重发 35（Qt 原逻辑）
     * - 获取期(Down=1005) → Qt 不做处理、链路停在等待 50 上，仅靠 999ms 定时器刷新界面；
     *   本实现等价处理：设备已确认命令且无数据帧可回 → 置 RX_Msg_End 并结束获取（见文件头注释）
     */
    onE5() {
        Logger.info('serial', 'RX E5 短应答', { state: stateTag() });
        const cmd = this.pendingCommand;
        if (cmd && cmd.expectE5) {
            this.resolvePending({ type: 'ack_e5' });
            return;
        }
        if (Qt.connecting && Qt.Down === 1001) {
            Qt.ErrorCounter++;
            Qt.Connect_Index = 0;
            if (Qt.ErrorCounter >= 5) {
                this.commError('通信错误（收到 E5 短应答，重试已超过 5 次）');
                return;
            }
            Logger.warn('serial', `连接期收到 E5，ErrorCounter=${Qt.ErrorCounter}，重发 35`);
            this.chainSend(1001, 0);
            return;
        }
        if (Qt.fetching && Qt.Down === 1005) {
            // 设备对获取命令仅回 E5（无数据帧）→ 获取链路到此结束
            Qt.RX_Msg_End = true;
            Logger.info('serial', '获取期收到 E5：设备确认命令且无数据帧可回，结束获取');
            Bus.emit('status', '设备无更多数据（E5 确认），获取结束');
            this.finishFetch();
            return;
        }
        Logger.warn('serial', '空闲状态收到孤立 E5，忽略');
    },

    /**
     * 收到 0xA2（Qt TeleUnpack 置 ACK2）：
     * Qt 的 ACK 分支对 ACK1/ACK2 同样处理——连接期计错误并重发；
     * 但不作为对话框 Apply 的成功标志（旧版 Qt 只判 ACK1）
     */
    onA2() {
        Logger.info('serial', 'RX A2 回显', { state: stateTag() });
        if (Qt.connecting && Qt.Down === 1001) {
            Qt.ErrorCounter++;
            Qt.Connect_Index = 0;
            if (Qt.ErrorCounter >= 5) {
                this.commError('通信错误（收到 A2 应答，重试已超过 5 次）');
                return;
            }
            Logger.warn('serial', `连接期收到 A2，ErrorCounter=${Qt.ErrorCounter}，重发 35`);
            this.chainSend(1001, 0);
        }
        // Qt: Down==1005 时 ACK2 无动作，此处同样忽略（仅记录）
    },

    // ---------------- 在线连接状态机（Down=1001） ----------------

    /**
     * 在线连接：对应 new1Dlg::OnConnect 的第一步（发 35 号同期参数回读命令）
     */
    async startConnect() {
        Qt.connecting = true;
        Qt.fetching = false;
        Qt.Down = 1001;
        Qt.ErrorCounter = 0;
        Qt.Connect_Index = 0;
        Qt.Msg_Len = 0;
        Qt.RX_Msg_End = true;
        Logger.info('serial', '开始在线连接（Down=1001）');
        Bus.emit('status', 'Connecting...... Please waitting!');
        // 发送同期参数命令（TQCS_block_35）
        await this.chainSend(1001, 0);
    },

    /**
     * 按状态机发下一条命令（对应 handleTimer11 底部的发送分支）
     */
    async chainSend(down, index) {
        let nr = null;
        if (down === 1001) {
            if (index === 0) nr = 35; // TQCS_block_35 同期参数
            else if (index === 1) nr = 33; // ACAD_block_33 AD参数
            else if (index === 2) nr = 37; // ADData_block_37 AD计算值
            else if (index === 3) nr = 47; // AC_angle_block_47 角度矢量
            else return;
        } else if (down === 1005) {
            if (index === 0) nr = 37;
            else if (index === 1 || index === 2) nr = 47; // Qt: index1 和 index2 都发 47
            else if (index === 3) nr = 49; // Message_block_49 通信报文
            else return;
        } else {
            return;
        }
        try {
            const frame = telePack(nr, [], 0);
            await this.write(frame);
            Logger.info('serial', `TX 报文 ${nr} (0x${nr.toString(16)})`, {
                hex: Logger.hex(frame),
                state: stateTag(),
            });
        } catch (e) {
            Logger.error('serial', `发送报文 ${nr} 失败`, e);
            this.commError(`发送报文 ${nr} 失败: ${e.message}`);
            return;
        }
        this.startStageTimer();
    },

    /**
     * 连接链路逐级推进
     */
    stepConnectChain(frame, known) {
        // Qt: TeleUnpack 成功后 Connect_Index++（数据帧到达即 +1）
        Qt.Connect_Index++;
        this.clearStageTimer();

        const nr = frame.telegramNr;
        if (!known) {
            // 未知帧（ErrorCounter 已在 onDataFrame 计入）
            this.retryConnect('返回命令错误');
            return;
        }
        if (nr === 34) {
            // 同期参数已加载，继续请求 AD 参数（Connect_Index==1）
            this.chainSend(1001, 1);
        } else if (nr === 32) {
            this.chainSend(1001, 2);
        } else if (nr === 36) {
            this.chainSend(1001, 3);
        } else if (nr === 48) {
            // 角度矢量已加载 —— Qt 旧版在此 break 并提示 "All data has been loaded!"
            this.finishConnect();
        } else {
            // 未知帧：Qt 中 ErrorCounter++ 后重发 35（Connect_Index=0）
            this.retryConnect('返回命令错误');
        }
    },

    retryConnect(reason) {
        Qt.Connect_Index = 0;
        if (Qt.ErrorCounter >= 5) {
            this.commError(`通信错误（${reason}，重试已超过 5 次）`);
            return;
        }
        Logger.warn('serial', `${reason}，ErrorCounter=${Qt.ErrorCounter}，重发 35 号命令`);
        this.chainSend(1001, 0);
    },

    /**
     * 连接完成：对应 Qt OnConnect 成功分支
     * Current_Style = AD1_ValidData[55]；DC_V_I_select 全置 1（Qt 20240618 版本行为）
     */
    finishConnect() {
        Qt.connecting = false;
        this.clearStageTimer();
        if (Qt.AD1_ValidData) {
            Qt.Current_Style = Qt.AD1_ValidData[55] || 0;
        }
        Qt.DC_V_I_select = [1, 1, 1, 1, 1, 1, 1, 1];
        Qt.Down = 1000;
        Logger.info('serial', '在线连接完成（All data has been loaded）', {
            Current_Style: Qt.Current_Style,
        });
        Bus.emit('status', 'All data has been loaded!');
        Bus.emit('connect-done');
    },

    // ---------------- 主界面获取状态机（Down=1005） ----------------

    /**
     * 主界面"获取信息"：对应 Dlg_ZJM::OnButtonZjmFetch
     * 先发 37（ADData_block_37），此后由状态机续发 47→47→49
     */
    async startFetch() {
        if (Qt.connecting) return;
        if (this.pendingCommand) {
            await showMessageBox('提示', '已有命令正在执行，请稍候');
            return;
        }
        Qt.fetching = true;
        Qt.Down = 1005;
        Qt.ErrorCounter = 0;
        Qt.Connect_Index = 0;
        Qt.RX_Msg_End = false; // Qt: 获取开始时置 false
        Qt.Msg_Len = 0;
        Logger.info('serial', '开始主界面获取（Down=1005）');
        await this.chainSend(1005, 0);
    },

    /**
     * 获取链路推进（严格对照 Qt handleTimer11 发送表）：
     *   Qt：TeleUnpack 成功即 Connect_Index++（任何有效数据帧都推进一步），
     *   未知报文号 → ErrorCounter++、Connect_Index=0；
     *   发送表（Down==1005）：
     *     Connect_Index==0 → 发 37；==1 → 发 47；==2 且 !RX_Msg_End → 发 47；
     *     ==3 且 !RX_Msg_End → 发 49；其余 → 链路自然结束
     *   3 号帧（告警列表无头）由 saveResponse 置 RX_Msg_End/Connect_Index=2，
     *   未完 → 续发 47 继续读列表；读完 → 结束
     */
    stepFetchChain(frame, known) {
        // Qt: TeleUnpack 成功后 Connect_Index++（数据帧到达即 +1）
        Qt.Connect_Index++;
        this.clearStageTimer();

        if (!known) {
            // Qt: 未知报文号 → ErrorCounter++（onDataFrame 已计入）、Connect_Index=0 → 重发 37
            Qt.Connect_Index = 0;
            if (Qt.ErrorCounter >= 5) {
                this.commError('通信错误（主界面获取重试超过 5 次）');
                return;
            }
            Logger.warn('serial', `获取期未知帧，ErrorCounter=${Qt.ErrorCounter}，重发 37 号命令`);
            this.chainSend(1005, 0);
            return;
        }

        const nr = frame.telegramNr;
        // 告警列表续帧（3 号）：saveResponse 已按 Qt 置 RX_Msg_End/Connect_Index；
        // 未完 → 续发 47 继续读列表（Qt：Connect_Index 被置回 2，发送表匹配 47 分支）
        if (nr === 3 && !Qt.RX_Msg_End) {
            this.chainSend(1005, 2);
            return;
        }

        // Qt handleTimer11 发送表（Down==1005）
        if (Qt.Connect_Index === 0) { this.chainSend(1005, 0); return; }
        if (Qt.Connect_Index === 1) { this.chainSend(1005, 1); return; }
        if (Qt.Connect_Index === 2 && !Qt.RX_Msg_End) { this.chainSend(1005, 2); return; }
        if (Qt.Connect_Index === 3 && !Qt.RX_Msg_End) { this.chainSend(1005, 3); return; }
        // 其余情况（收到 50、RX_Msg_End 已置位等）：链路自然结束
        this.finishFetch();
    },

    finishFetch() {
        Qt.fetching = false;
        this.clearStageTimer();
        // Qt: Down 保持 1005（updateData 以 Down==1005 为条件刷新消息表格）
        Logger.info('serial', '主界面获取结束', {
            state: stateTag(),
            ADCacul: Qt.ADCacul_ValidData ? Qt.ADCacul_ValidData.length : null,
            BaseAngle: Qt.BaseAngle_ValidData ? Qt.BaseAngle_ValidData.length : null,
            Message: Qt.Message_ValidData ? Qt.Message_ValidData.length : null,
        });
        Bus.emit('fetch-done');
    },

    // ---------------- 对话框命令 ----------------

    /**
     * 发送单条命令并等待响应（对应各对话框 Fetch/Apply + 全局分发）
     * @param {object} opt { nr, data, zero, f2, expectNr, expectNrs, expectE5, timeout, down }
     *   - expectNr / expectNrs: 期待的数据帧报文号（响应含 4 字节头，resolve(frame)）；
     *     expectNrs 为数组时可接受多种响应号（如校准命令装置回 39、Qt 假定 41）
     *   - expectE5: true 时收到 E5 即成功（Qt Apply 类命令以 ACK1 为成功标志）
     * @returns {Promise<object>}
     */
    sendCommand(opt) {
        if (!Qt.serial.connected) {
            return Promise.reject(new Error('串口未连接'));
        }
        if (Qt.connecting || Qt.fetching) {
            return Promise.reject(new Error('正在执行连接/获取流程，请稍候'));
        }
        if (this.pendingCommand) {
            return Promise.reject(new Error('已有命令正在执行，请稍候'));
        }

        Qt.Down = opt.down != null ? opt.down : Qt.Down;

        return new Promise((resolve, reject) => {
            const frame = telePack(opt.nr, opt.data || [], opt.zero || 0, 0, opt.f2 != null ? opt.f2 : 0x80);
            const expect = opt.expectNrs || (opt.expectNr != null ? [opt.expectNr] : null);
            const cmd = {
                nr: opt.nr,
                expectNr: opt.expectNr,
                expectNrs: opt.expectNrs,
                expectE5: !!opt.expectE5,
                down: Qt.Down,
                timer: null,
                resolve: (f) => {
                    if (this.pendingCommand !== cmd) return; // 防止残留 timer 误杀新命令
                    clearTimeout(cmd.timer);
                    this.pendingCommand = null;
                    this.clearStageTimer();
                    resolve(f);
                },
                reject: (err) => {
                    if (this.pendingCommand !== cmd) return;
                    clearTimeout(cmd.timer);
                    this.pendingCommand = null;
                    this.clearStageTimer();
                    reject(err);
                },
            };
            this.pendingCommand = cmd;
            cmd.timer = setTimeout(() => {
                // 用对象同一性判断：若已 resolve/reject（或已被新命令取代）则不做任何事
                if (this.pendingCommand === cmd) {
                    const expTxt = expect ? expect.join('/') : 'E5';
                    Logger.warn('serial', `对话框命令等待响应超时（报文 ${opt.nr}，期待 ${expTxt}）`);
                    cmd.reject(new Error(`等待响应超时（报文 ${opt.nr}，期待 ${expTxt}）`));
                }
            }, opt.timeout || this.COMM_TIMEOUT);
            this.write(frame).then(() => {
                Logger.info('serial', `对话框命令 TX：${opt.nr} (0x${opt.nr.toString(16)})`, {
                    hex: Logger.hex(frame),
                    expect: expect ? `Nr=${expect.join('/')}` : 'E5',
                    down: Qt.Down,
                });
            }).catch((e) => {
                Logger.error('serial', `对话框命令发送失败：${opt.nr}`, e);
                cmd.reject(e);
            });
        });
    },

    resolvePending(value) {
        if (this.pendingCommand) {
            Logger.debug('serial', `对话框命令完成（Nr=${this.pendingCommand.nr}）`);
            this.pendingCommand.resolve(value);
        }
    },
    rejectPending(reason) {
        if (this.pendingCommand) {
            Logger.debug('serial', `对话框命令被拒绝（Nr=${this.pendingCommand.nr}）：${reason}`);
            this.pendingCommand.reject(new Error(reason));
        }
    },

    // ---------------- 超时与错误 ----------------

    startStageTimer() {
        this.clearStageTimer();
        this.stageTimer = setTimeout(() => {
            Logger.error('serial', `通信超时（${this.COMM_TIMEOUT}ms）`, { state: stateTag() });
            if (Qt.connecting && Qt.Down === 1001) {
                // Qt handleTimer12: 弹错误并关闭
                this.commError('communication error（通信超时）');
            } else if (Qt.fetching && Qt.Down === 1005) {
                // Qt handleTimer12 对获取流程只弹错误不重试；本实现按"每级超时"设计重发 37，
                // 但每次重试计入 ErrorCounter（Qt 判据 ErrorCounter>4 报通信错误），
                // 避免设备不响应时无限循环（见文件头注释）
                Qt.Connect_Index = 0;
                Qt.ErrorCounter++;
                if (Qt.ErrorCounter >= 5) {
                    this.commError('communication error（主界面获取超时，重试超过 5 次）');
                } else {
                    Logger.warn('serial', `获取超时第 ${Qt.ErrorCounter} 次，重发 37 号命令`);
                    this.chainSend(1005, 0);
                }
            }
            // 对话框命令超时由 sendCommand 内部 timer 处理
        }, this.COMM_TIMEOUT);
    },

    clearStageTimer() {
        if (this.stageTimer) {
            clearTimeout(this.stageTimer);
            this.stageTimer = null;
        }
    },

    async commError(msg) {
        this.clearStageTimer();
        Qt.connecting = false;
        Qt.fetching = false;
        Logger.error('serial', '通信错误：' + msg, { state: stateTag() });
        Bus.emit('status', msg);
        // 串口保持打开（区别于 Qt 直接退出），给出原因与后续操作建议
        let tip = msg;
        if (Qt.serial.connected) {
            tip += '（串口仍保持打开。装置未连接、未上电或波特率不符时会出现此错误；可再次点击"在线连接"重试，或点击"离线连接"进入主界面）';
        }
        await showMessageBox('错误', tip);
    },

    /**
     * 退出（对应 new1Dlg::OnExit）
     */
    async exit() {
        Logger.info('serial', '退出程序（关闭串口）');
        if (this.port) await this.disconnect();
        Qt.Down = 1000;
    },
};

/** 状态机关键状态快照（日志明细用） */
function stateTag() {
    return `Down=${Qt.Down} idx=${Qt.Connect_Index} err=${Qt.ErrorCounter} msgEnd=${Qt.RX_Msg_End} msgLen=${Qt.Msg_Len}`;
}
