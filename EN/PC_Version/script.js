// 动作序列设计器 JavaScript
class ActionSequenceDesigner {
    constructor() {
        this.actions = [];
        this.selectedStructure = null;
        this.zoomLevel = 1;
        this.timelineScale = 100; // 像素/秒
        this.isPlaying = false;
        this.currentTime = 0;
        this.playbackInterval = null;
        this.editingAction = null;
        
        // Web Serial API 支持
        this.port = null;
        this.reader = null;
        this.writer = null;
        this.isConnected = false;
        
        this.init();
    }

    init() {
        this.bindEvents();
        this.createTimeline();
        this.updateStatus();
        this.updateCodeOutput();
        // 初始化参数标签为默认状态
        this.updateParameterLabels(null);
        // 清除所有机器人部位的激活状态
        document.querySelectorAll('.robot-part').forEach(part => {
            part.classList.remove('active');
        });
    }

    bindEvents() {
        // 结构选择
        document.querySelectorAll('.structure-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                // 确保获取到正确的结构名称，即使点击的是子元素
                const structure = e.target.dataset.structure || e.currentTarget.dataset.structure;
                this.selectStructure(structure);
            });
        });

        // 添加动作
        document.getElementById('addAction').addEventListener('click', () => {
            this.addAction();
        });

        // 控制按钮
        document.getElementById('playSequence').addEventListener('click', () => {
            this.playSequence();
        });

        document.getElementById('pauseSequence').addEventListener('click', () => {
            this.pauseSequence();
        });

        document.getElementById('stopSequence').addEventListener('click', () => {
            this.stopSequence();
        });

        document.getElementById('resetAll').addEventListener('click', () => {
            this.resetAll();
        });

        document.getElementById('clearSequence').addEventListener('click', () => {
            this.clearSequence();
        });

        document.getElementById('exportCode').addEventListener('click', () => {
            this.exportCode();
        });

        // 时间轴控制
        document.getElementById('zoomIn').addEventListener('click', () => {
            this.zoomIn();
        });

        document.getElementById('zoomOut').addEventListener('click', () => {
            this.zoomOut();
        });

        document.getElementById('fitToView').addEventListener('click', () => {
            this.fitToView();
        });

        // 模态框事件
        document.querySelector('.close').addEventListener('click', () => {
            this.closeModal();
        });

        document.getElementById('saveAction').addEventListener('click', () => {
            this.saveAction();
        });

        document.getElementById('deleteAction').addEventListener('click', () => {
            this.deleteAction();
        });

        document.getElementById('cancelEdit').addEventListener('click', () => {
            this.closeModal();
        });

        // 自定义下拉菜单事件
        const editStructureHeader = document.getElementById('editStructureHeader');
        const editStructureOptions = document.getElementById('editStructureOptions');
        const editStructureContainer = document.getElementById('editStructureContainer');

        if (editStructureHeader) {
            editStructureHeader.addEventListener('click', () => {
                editStructureContainer.classList.toggle('open');
                editStructureOptions.style.display = editStructureOptions.style.display === 'none' ? 'block' : 'none';
            });
        }

        // 点击选项时更新选择
        document.querySelectorAll('.custom-option').forEach(option => {
            option.addEventListener('click', (e) => {
                const value = e.currentTarget.dataset.value;
                const text = e.currentTarget.textContent.trim();
                
                // 更新隐藏的input值
                document.getElementById('editStructure').value = value;
                
                // 更新显示的选项
                const selectedOption = document.querySelector('.selected-option');
                selectedOption.innerHTML = e.currentTarget.innerHTML;
                
                // 关闭下拉菜单
                editStructureContainer.classList.remove('open');
                editStructureOptions.style.display = 'none';
                
                // 更新模态框中的参数标签
                this.updateModalParameterLabels(value);
            });
        });

        // 点击外部关闭下拉菜单
        document.addEventListener('click', (e) => {
            if (!editStructureContainer.contains(e.target)) {
                editStructureContainer.classList.remove('open');
                editStructureOptions.style.display = 'none';
            }
        });

        // 模态框中的结构选择
        document.getElementById('editStructure').addEventListener('change', (e) => {
            this.updateModalParameterLabels(e.target.value);
        });

        // Web Serial API 连接按钮
        this.addSerialConnectionButton();
    }

    // 添加串口连接按钮
    addSerialConnectionButton() {
        const statusBar = document.querySelector('.status-bar');
        const connectButton = document.createElement('button');
        connectButton.className = 'btn btn-primary';
        connectButton.id = 'connectSerial';
        connectButton.textContent = '🔌 连接Arduino';
        connectButton.addEventListener('click', () => {
            this.connectToArduino();
        });
        
        statusBar.appendChild(connectButton);
    }

    // 连接Arduino
    async connectToArduino() {
        try {
            // 检查Web Serial API是否可用
            if (!('serial' in navigator)) {
                alert('您的浏览器不支持Web Serial API。请使用Chrome或Edge浏览器。');
                return;
            }

            // 请求串口访问权限
            this.port = await navigator.serial.requestPort({
                filters: [
                    { usbVendorId: 0x2341 }, // Arduino
                    { usbVendorId: 0x2a03 }, // Arduino.org
                    { usbVendorId: 0x0403 }, // FTDI
                    { usbVendorId: 0x1a86 }, // CH340
                    { usbVendorId: 0x10c4 }  // CP210x
                ]
            });

            // 打开串口
            await this.port.open({ baudRate: 9600 });
            
            // 设置读取器和写入器
            this.reader = this.port.readable.getReader();
            this.writer = this.port.writable.getWriter();
            
            this.isConnected = true;
            this.updateConnectionStatus(true);
            
            // 开始读取数据
            this.readSerialData();
            
            console.log('✅ 已连接到Arduino');
            
        } catch (error) {
            console.error('连接失败:', error);
            alert('连接失败: ' + error.message);
            this.updateConnectionStatus(false);
        }
    }

    // 读取串口数据
    async readSerialData() {
        if (!this.reader) return;
        
        try {
            while (this.port && this.port.readable) {
                const { value, done } = await this.reader.read();
                if (done) break;
                
                // 将Uint8Array转换为字符串
                const text = new TextDecoder().decode(value);
                this.handleSerialResponse(text);
            }
        } catch (error) {
            console.error('读取串口数据失败:', error);
            this.updateConnectionStatus(false);
        }
    }

    // 处理串口响应
    handleSerialResponse(response) {
        console.log('Arduino响应:', response);
        
        // 更新状态显示
        const codeOutput = document.getElementById('codeOutput');
        if (codeOutput) {
            const currentText = codeOutput.textContent;
            codeOutput.textContent = currentText + '\n// Arduino: ' + response.trim();
            codeOutput.scrollTop = codeOutput.scrollHeight;
        }
    }

    // 更新连接状态
    updateConnectionStatus(connected) {
        const statusIndicator = document.getElementById('connectionStatus');
        const connectionText = document.getElementById('connectionText');
        const connectButton = document.getElementById('connectSerial');
        
        if (connected) {
            statusIndicator.classList.remove('disconnected');
            connectionText.textContent = '已连接';
            if (connectButton) {
                connectButton.textContent = '🔌 断开连接';
                connectButton.className = 'btn btn-danger';
                connectButton.onclick = () => this.disconnectFromArduino();
            }
        } else {
            statusIndicator.classList.add('disconnected');
            connectionText.textContent = '未连接';
            if (connectButton) {
                connectButton.textContent = '🔌 连接Arduino';
                connectButton.className = 'btn btn-primary';
                connectButton.onclick = () => this.connectToArduino();
            }
        }
    }

    // 断开连接
    async disconnectFromArduino() {
        try {
            if (this.reader) {
                await this.reader.cancel();
                this.reader.releaseLock();
                this.reader = null;
            }
            if (this.writer) {
                this.writer.releaseLock();
                this.writer = null;
            }
            if (this.port) {
                await this.port.close();
                this.port = null;
            }
            
            this.isConnected = false;
            this.updateConnectionStatus(false);
            console.log('✅ 已断开Arduino连接');
            
        } catch (error) {
            console.error('断开连接失败:', error);
        }
    }

    // 发送命令到Arduino
    async sendCommand(command) {
        if (!this.isConnected || !this.writer) {
            console.log('未连接到Arduino，命令:', command);
            return false;
        }
        
        try {
            // 添加换行符
            const commandWithNewline = command + '\n';
            const encoder = new TextEncoder();
            const data = encoder.encode(commandWithNewline);
            
            await this.writer.write(data);
            console.log('✅ 已发送命令到Arduino:', command);
            return true;
            
        } catch (error) {
            console.error('发送命令失败:', error);
            this.updateConnectionStatus(false);
            return false;
        }
    }

    selectStructure(structure) {
        console.log('选择结构:', structure);
        // 移除之前的选中状态
        document.querySelectorAll('.structure-btn').forEach(btn => {
            btn.classList.remove('active');
        });

        // 移除之前SVG部位的激活状态
        document.querySelectorAll('.robot-part').forEach(part => {
            part.classList.remove('active');
        });

        // 设置新的选中状态
        const btn = document.querySelector(`[data-structure="${structure}"]`);
        if (btn) {
            btn.classList.add('active');
            this.selectedStructure = structure;
            console.log('结构选择成功:', structure);
            
            // 根据选中的结构更新参数名称
            this.updateParameterLabels(structure);
            
            // 更新SVG部位颜色
            this.updateRobotPartHighlight(structure);
        } else {
            console.error('找不到结构按钮:', structure);
        }
    }

    // 根据选中的结构更新参数名称
    updateParameterLabels(structure) {
        // 查找前方角度和弯曲角度的标签
        const forwardLabel = document.querySelector('label[for="forwardAngle"]');
        const bendLabel = document.querySelector('label[for="bendAngle"]');
        
        if (forwardLabel && bendLabel) {
            if (!structure) {
                // 默认状态
                forwardLabel.textContent = '前方角度 (F)';
                bendLabel.textContent = '弯曲角度 (R)';
                return;
            }
            
            switch(structure) {
                case 'LE':
                case 'RE':
                    forwardLabel.textContent = '转动角度 - F（逆时针为正）';
                    bendLabel.textContent = '俯仰角度 - R（上扬为正）';
                    break;
                case 'HD':
                    forwardLabel.textContent = '转动角度 - F（逆时针为正）';
                    bendLabel.textContent = '俯仰角度 - R（上扬为正）';
                    break;
                case 'LF':
                case 'RF':
                case 'LR':
                case 'RR':
                    forwardLabel.textContent = '抬起/放下 - F（抬起为正）';
                    bendLabel.textContent = '弯曲角度 - R（弯曲为正）';
                    break;
                case 'TA':
                    forwardLabel.textContent = '转动角度（逆时针为正）';
                    bendLabel.textContent = '弯曲角度（上扬为正）';
                    break;
                default:
                    forwardLabel.textContent = '前方角度 (F)';
                    bendLabel.textContent = '弯曲角度 (R)';
                    break;
            }
        }
    }

    // 根据选中的结构更新模态框中的参数名称
    updateModalParameterLabels(structure) {
        const forwardLabel = document.querySelector('label[for="editForwardAngle"]');
        const bendLabel = document.querySelector('label[for="editBendAngle"]');
        
        if (structure === 'HD') {
            forwardLabel.textContent = '左右角度 (L)';
            bendLabel.textContent = '上下角度 (U)';
        } else if (structure === 'TA') {
            forwardLabel.textContent = '左右角度 (L)';
            bendLabel.textContent = '上下角度 (U)';
        } else if (structure === 'LE' || structure === 'RE') {
            forwardLabel.textContent = '前后角度 (F)';
            bendLabel.textContent = '左右角度 (L)';
        } else {
            forwardLabel.textContent = '前方角度 (F)';
            bendLabel.textContent = '弯曲角度 (R)';
        }
    }

    updateCustomSelectDisplay(structure) {
        const selectedOption = document.querySelector('.selected-option');
        const customOptions = document.querySelectorAll('.custom-option');
        
        // 找到对应的选项
        const targetOption = Array.from(customOptions).find(option => 
            option.dataset.value === structure
        );
        
        if (targetOption && selectedOption) {
            selectedOption.innerHTML = targetOption.innerHTML;
        }
    }

    addAction() {
        if (!this.selectedStructure) {
            alert('请先选择结构');
            return;
        }

        const forwardAngle = parseInt(document.getElementById('forwardAngle').value);
        const bendAngle = parseInt(document.getElementById('bendAngle').value);
        const speed = parseInt(document.getElementById('speedLevel').value);
        const delayTime = parseInt(document.getElementById('delayTime').value);

        const action = {
            id: Date.now() + Math.random(), // 生成唯一ID
            structure: this.selectedStructure,
            forwardAngle: forwardAngle,
            bendAngle: bendAngle,
            speed: speed,
            delayTime: delayTime,
            startTime: this.calculateStartTime(),
            duration: this.calculateDuration(forwardAngle, bendAngle, speed),
            sent: false // 添加发送状态标记
        };

        this.actions.push(action);
        this.updateTimeline();
        this.updateStatus();
        this.updateCodeOutput();

        console.log(`✅ 添加动作: ${action.structure} F${forwardAngle} R${bendAngle} S${speed}`);
    }

    calculateStartTime() {
        if (this.actions.length === 0) return 0;

        const currentStructure = this.selectedStructure;
        const delayTime = parseInt(document.getElementById('delayTime').value);
        
        // 找到同一结构的最后一个动作
        const sameStructureActions = this.actions.filter(action => action.structure === currentStructure);
        
        if (sameStructureActions.length === 0) {
            // 这是这个结构的第一个动作，从0开始
            return delayTime;
        }
        
        // 找到同一结构的最后一个动作
        const lastSameStructureAction = sameStructureActions[sameStructureActions.length - 1];
        
        // 同一结构的动作串行执行，加上延迟时间
        return lastSameStructureAction.startTime + lastSameStructureAction.duration + delayTime;
    }

    calculateDuration(forwardAngle, bendAngle, speed) {
        const speedLevels = {1: 20, 2: 15, 3: 10, 4: 5, 5: 2};
        const baseDuration = speedLevels[speed] || 15;
        
        // 根据角度变化计算步数，每步的延迟时间
        const maxAngle = Math.max(Math.abs(forwardAngle), Math.abs(bendAngle));
        const steps = Math.max(maxAngle, 1);
        
        return steps * baseDuration;
    }

    createTimeline() {
        this.createTimelineMarkers();
        this.createTimelineTracks();
    }

    createTimelineMarkers() {
        const markersContainer = document.getElementById('timelineMarkers');
        markersContainer.innerHTML = '';

        const maxTime = Math.max(...this.actions.map(a => a.startTime + a.duration), 5000);
        const markerCount = Math.ceil(maxTime / 1000);

        for (let i = 0; i <= markerCount; i++) {
            const marker = document.createElement('div');
            marker.className = 'timeline-marker';
            marker.textContent = `${i}s`;
            markersContainer.appendChild(marker);
        }
    }

    createTimelineTracks() {
        const tracksContainer = document.getElementById('timelineTracks');
        if (!tracksContainer) {
            console.error('找不到时间轴轨道容器');
            return;
        }
        tracksContainer.innerHTML = '';

        const structures = ['RF', 'LF', 'RR', 'LR', 'TA', 'RE', 'LE', 'HD'];
        const structureNames = {
            'RF': '右前脚', 'LF': '左前脚', 'RR': '右后脚', 'LR': '左后脚',
            'TA': '尾巴', 'RE': '右耳', 'LE': '左耳', 'HD': '头部'
        };

        structures.forEach(structure => {
            const track = document.createElement('div');
            track.className = 'timeline-track';
            track.innerHTML = `
                <div class="track-label">${structureNames[structure]}</div>
                <div class="track-content" data-structure="${structure}"></div>
            `;
            tracksContainer.appendChild(track);
            console.log(`创建轨道: ${structure} - ${structureNames[structure]}`);
        });
        
        // 验证轨道创建
        setTimeout(() => {
            console.log('轨道创建验证:');
            structures.forEach(structure => {
                const trackContent = document.querySelector(`.timeline-tracks [data-structure="${structure}"]`);
                if (trackContent) {
                    console.log(`✓ ${structure}: 轨道容器存在, 宽度=${trackContent.offsetWidth}px`);
                } else {
                    console.error(`✗ ${structure}: 轨道容器不存在`);
                }
            });
        }, 100);
    }

    // 更新时间轴
    updateTimeline() {
        this.updateActionBlocks();
        this.createTimelineMarkers();
    }

    // 更新动作块
    updateActionBlocks() {
        // 清除现有的动作块
        document.querySelectorAll('.action-block').forEach(block => {
            block.remove();
        });
        
        // 重新渲染所有动作块
        this.renderActionBlocks();
    }

    // 渲染动作块
    renderActionBlocks() {
        // 清除现有的动作块
        document.querySelectorAll('.action-block').forEach(block => {
            block.remove();
        });
        
        // 为每个动作创建块
        this.actions.forEach(action => {
            const block = this.createActionBlock(action);
            
            // 找到对应的轨道容器
            const trackContent = document.querySelector(`.timeline-tracks [data-structure="${action.structure}"]`);
            if (trackContent) {
                trackContent.appendChild(block);
            } else {
                console.error(`找不到结构 ${action.structure} 的轨道容器`);
            }
        });
    }

    // 创建动作块
    createActionBlock(action) {
        const block = document.createElement('div');
        block.className = `action-block ${action.structure.toLowerCase()}`;
        block.dataset.actionId = action.id; // 添加唯一ID
        
        const startTime = this.formatTime(action.startTime);
        const duration = this.formatTime(action.duration);
        
        block.innerHTML = `
            <div class="drag-handle">⋮⋮</div>
            <div class="action-info">
                <div class="action-title">${action.structure} F${action.forwardAngle} R${action.bendAngle}</div>
                <div class="action-time">
                    <div class="start-time">开始: ${startTime}</div>
                    <div class="duration-time">执行: ${duration}</div>
                </div>
            </div>
            <div class="resize-handle"></div>
        `;
        
        // 设置位置和大小
        const left = (action.startTime / 1000) * this.timelineScale * this.zoomLevel;
        const width = (action.duration / 1000) * this.timelineScale * this.zoomLevel;
        
        block.style.left = `${left}px`;
        block.style.width = `${Math.max(width, 80)}px`; // 最小宽度80px
        
        // 使动作块可拖拽和调整大小
        this.makeDraggable(block, action);
        this.makeResizable(block, action);
        
        // 添加点击事件用于编辑
        block.addEventListener('dblclick', () => {
            this.editAction(action);
        });
        
        return block;
    }

    makeDraggable(block, action) {
        let isDragging = false;
        let startX, startLeft;

        const dragHandle = block.querySelector('.drag-handle');
        dragHandle.addEventListener('mousedown', (e) => {
            isDragging = true;
            startX = e.clientX;
            startLeft = parseFloat(block.style.left) || 0;
            e.preventDefault();
        });

        document.addEventListener('mousemove', (e) => {
            if (!isDragging) return;

            const deltaX = e.clientX - startX;
            const newLeft = Math.max(0, startLeft + deltaX);
            block.style.left = `${newLeft}px`;

            // 更新时间
            const newStartTime = (newLeft / (this.timelineScale * this.zoomLevel)) * 1000;
            action.startTime = newStartTime;
            
            // 更新时间显示
            this.updateActionTimeDisplay(block, action);
            
            this.updateCodeOutput();
        });

        document.addEventListener('mouseup', () => {
            isDragging = false;
        });
    }

    makeResizable(block, action) {
        let isResizing = false;
        let startX, startWidth;

        const resizeHandle = block.querySelector('.resize-handle');
        resizeHandle.addEventListener('mousedown', (e) => {
            isResizing = true;
            startX = e.clientX;
            startWidth = parseFloat(block.style.width) || 60;
            e.preventDefault();
        });

        document.addEventListener('mousemove', (e) => {
            if (!isResizing) return;

            const deltaX = e.clientX - startX;
            const newWidth = Math.max(60, startWidth + deltaX);
            block.style.width = `${newWidth}px`;

            // 更新持续时间
            const newDuration = (newWidth / (this.timelineScale * this.zoomLevel)) * 1000;
            action.duration = newDuration;
            
            // 更新时间显示
            this.updateActionTimeDisplay(block, action);
            
            this.updateCodeOutput();
        });

        document.addEventListener('mouseup', () => {
            isResizing = false;
        });
    }

    editAction(action) {
        this.editingAction = action;
        
        // 填充编辑表单
        document.getElementById('editStructure').value = action.structure;
        document.getElementById('editForwardAngle').value = action.forwardAngle;
        document.getElementById('editBendAngle').value = action.bendAngle;
        document.getElementById('editSpeedLevel').value = action.speed;
        document.getElementById('editDelayTime').value = action.delayTime;

        // 更新自定义下拉菜单的显示
        this.updateCustomSelectDisplay(action.structure);

        // 更新模态框中的参数标签
        this.updateModalParameterLabels(action.structure);

        // 显示模态框
        document.getElementById('actionModal').style.display = 'block';
    }

    saveAction() {
        if (!this.editingAction) return;

        // 更新动作数据
        this.editingAction.structure = document.getElementById('editStructure').value;
        this.editingAction.forwardAngle = parseInt(document.getElementById('editForwardAngle').value);
        this.editingAction.bendAngle = parseInt(document.getElementById('editBendAngle').value);
        this.editingAction.speed = parseInt(document.getElementById('editSpeedLevel').value);
        this.editingAction.delayTime = parseInt(document.getElementById('editDelayTime').value);

        this.closeModal();
        this.updateActionBlocks();
        this.updateCodeOutput();
    }

    deleteAction() {
        if (!this.editingAction) return;

        const index = this.actions.findIndex(a => a.id === this.editingAction.id);
        if (index > -1) {
            this.actions.splice(index, 1);
        }

        this.closeModal();
        this.updateActionBlocks();
        this.updateStatus();
        this.updateCodeOutput();
    }

    closeModal() {
        document.getElementById('actionModal').style.display = 'none';
        this.editingAction = null;
    }

    // 播放序列
    playSequence() {
        if (this.actions.length === 0) {
            alert('请先添加动作');
            return;
        }

        this.isPlaying = true;
        this.currentTime = 0;
        
        // 清空之前的播放
        if (this.playbackInterval) {
            clearInterval(this.playbackInterval);
        }

        // 开始播放
        this.playbackInterval = setInterval(() => {
            this.currentTime += 100; // 100ms步进
            
            // 高亮当前执行的动作
            this.highlightCurrentActions();
            
            // 检查是否有动作需要执行
            const currentActions = this.actions.filter(action => 
                action.startTime <= this.currentTime && 
                action.startTime + action.duration >= this.currentTime
            );
            
            // 发送当前动作到Arduino
            currentActions.forEach(action => {
                if (!action.sent) {
                    const command = this.generateActionCommand(action);
                    this.sendCommand(command);
                    action.sent = true;
                }
            });
            
            // 检查是否播放完成
            if (this.currentTime >= this.getTotalDuration()) {
                this.stopSequence();
            }
        }, 100);
    }

    // 生成动作命令
    generateActionCommand(action) {
        let command = `${action.structure} F${action.forwardAngle} R${action.bendAngle} S${action.speed}`;
        
        if (action.delayTime > 0) {
            command += ` D${action.delayTime}`;
        }
        
        return command;
    }

    // 暂停序列
    pauseSequence() {
        this.isPlaying = false;
        if (this.playbackInterval) {
            clearInterval(this.playbackInterval);
            this.playbackInterval = null;
        }
        this.clearHighlights();
    }

    // 停止序列
    stopSequence() {
        this.isPlaying = false;
        this.currentTime = 0;
        if (this.playbackInterval) {
            clearInterval(this.playbackInterval);
            this.playbackInterval = null;
        }
        this.clearHighlights();
        
        // 重置所有动作的发送状态
        this.actions.forEach(action => {
            action.sent = false;
        });
    }

    // 高亮当前执行的动作
    highlightCurrentActions() {
        // 清除之前的高亮
        this.clearHighlights();
        
        // 找到当前时间正在执行的动作
        const currentActions = this.actions.filter(action => 
            action.startTime <= this.currentTime && 
            action.startTime + action.duration >= this.currentTime
        );
        
        // 高亮当前执行的动作块
        currentActions.forEach(action => {
            const block = document.querySelector(`[data-action-id="${action.id}"]`);
            if (block) {
                block.classList.add('playing');
                block.style.boxShadow = '0 0 10px rgba(79, 172, 254, 0.8)';
                
                // 高亮对应的机器人部位
                this.updateRobotPartHighlight(action.structure);
            }
        });
        
        // 标记已发送的动作
        this.actions.forEach(action => {
            if (action.sent) {
                const block = document.querySelector(`[data-action-id="${action.id}"]`);
                if (block) {
                    block.classList.add('sent');
                }
            }
        });
    }

    // 清除高亮
    clearHighlights() {
        document.querySelectorAll('.action-block').forEach(block => {
            block.classList.remove('playing');
            block.style.boxShadow = '';
        });
        
        // 清除机器人部位高亮
        document.querySelectorAll('.robot-part').forEach(part => {
            part.classList.remove('active');
        });
    }

    // 复位所有舵机
    async resetAll() {
        const command = 'q';
        const success = await this.sendCommand(command);
        
        if (success) {
            console.log('✅ 已发送复位命令');
            // 清空动作序列
            this.clearSequence();
        } else {
            console.log('❌ 发送复位命令失败');
        }
    }

    // 清空序列
    clearSequence() {
        this.actions = [];
        this.updateTimeline();
        this.updateStatus();
        this.updateCodeOutput();
        
        // 清除所有动作块的发送状态
        document.querySelectorAll('.action-block').forEach(block => {
            block.classList.remove('sent');
        });
    }

    // 导出代码
    exportCode() {
        const code = this.generateCode();
        if (code) {
            // 复制到剪贴板
            navigator.clipboard.writeText(code).then(() => {
                alert('代码已复制到剪贴板！');
            }).catch(() => {
                // 如果剪贴板API不可用，显示代码
                prompt('请复制以下代码:', code);
            });
        } else {
            alert('没有可导出的代码');
        }
    }

    generateCode() {
        if (this.actions.length === 0) return '';

        return this.actions.map(action => {
            let code = `${action.structure} F${action.forwardAngle} R${action.bendAngle} S${action.speed}`;
            
            if (action.delayTime > 0) {
                code += ` D${action.delayTime}`;
            }
            
            return code;
        }).join(';');
    }

    updateCodeOutput() {
        const code = this.generateCode();
        document.getElementById('codeOutput').textContent = code || '// 暂无动作序列';
    }

    updateStatus() {
        document.getElementById('actionCount').textContent = this.actions.length;
        document.getElementById('totalDuration').textContent = `${this.getTotalDuration()}ms`;
    }

    getTotalDuration() {
        if (this.actions.length === 0) return 0;
        
        const maxEndTime = Math.max(...this.actions.map(a => a.startTime + a.duration));
        return maxEndTime;
    }

    zoomIn() {
        this.zoomLevel = Math.min(this.zoomLevel * 1.5, 5);
        this.updateActionBlocks();
    }

    zoomOut() {
        this.zoomLevel = Math.max(this.zoomLevel / 1.5, 0.1);
        this.updateActionBlocks();
    }

    fitToView() {
        const maxTime = this.getTotalDuration();
        const containerWidth = document.querySelector('.timeline').offsetWidth - 200; // 减去标签宽度
        this.zoomLevel = containerWidth / (maxTime / 1000) / this.timelineScale;
        this.updateActionBlocks();
    }

    updateActionTimeDisplay(block, action) {
        const startTimeText = this.formatTime(action.startTime);
        const durationText = this.formatTime(action.duration);
        
        const timeDisplay = block.querySelector('.action-time');
        if (timeDisplay) {
            timeDisplay.innerHTML = `
                <div class="start-time">开始: ${startTimeText}</div>
                <div class="duration-time">执行: ${durationText}</div>
            `;
        }
    }

    undo() {
        // 实现撤销功能
        console.log('撤销');
    }

    redo() {
        // 实现重做功能
        console.log('重做');
    }

    formatTime(ms) {
        if (ms < 1000) {
            return `${ms}ms`;
        }
        
        const totalSeconds = Math.floor(ms / 1000);
        const minutes = Math.floor(totalSeconds / 60);
        const seconds = totalSeconds % 60;
        const milliseconds = ms % 1000;

        let timeString = '';
        if (minutes > 0) {
            timeString += `${minutes}m `;
        }
        if (seconds > 0 || minutes > 0) {
            timeString += `${seconds}s`;
        }
        if (milliseconds > 0 && totalSeconds === 0) {
            timeString += ` ${milliseconds}ms`;
        }
        
        return timeString.trim() || '0ms';
    }

    // 更新机器人部位高亮
    updateRobotPartHighlight(structure) {
        let targetPart = null;
        
        switch(structure) {
            case 'HD':
                targetPart = document.getElementById('head');
                break;
            case 'TA':
                targetPart = document.getElementById('tail');
                break;
            case 'LE':
                targetPart = document.getElementById('left-ear');
                break;
            case 'RE':
                targetPart = document.getElementById('right-ear');
                break;
            case 'LF':
                targetPart = document.getElementById('left-front-leg');
                break;
            case 'RF':
                targetPart = document.getElementById('right-front-leg');
                break;
            case 'LR':
                targetPart = document.getElementById('left-back-leg');
                break;
            case 'RR':
                targetPart = document.getElementById('right-back-leg');
                break;
            default:
                console.log('未知结构:', structure);
                return;
        }
        
        if (targetPart) {
            targetPart.classList.add('active');
            console.log('激活机器人部位:', structure);
        } else {
            console.error('找不到对应的机器人部位:', structure);
        }
    }
}

// 初始化应用
document.addEventListener('DOMContentLoaded', () => {
    window.actionDesigner = new ActionSequenceDesigner();
    
    // 添加测试按钮
    const testBtn = document.createElement('button');
    testBtn.textContent = '🧪 测试时间轴';
    testBtn.style.position = 'fixed';
    testBtn.style.top = '10px';
    testBtn.style.right = '10px';
    testBtn.style.zIndex = '1000';
    testBtn.onclick = () => {
        console.log('=== 时间轴测试 ===');
        console.log('轨道容器:', document.getElementById('timelineTracks'));
        console.log('轨道数量:', document.querySelectorAll('.timeline-track').length);
        console.log('轨道内容:', document.querySelectorAll('.track-content').length);
        console.log('动作块数量:', document.querySelectorAll('.action-block').length);
        
        // 测试添加一个动作
        const designer = window.actionDesigner;
        if (designer) {
            designer.selectStructure('RF');
            designer.addAction();
        }
    };
    document.body.appendChild(testBtn);
});

// 添加一些示例数据用于演示
function loadExample() {
    const designer = window.actionDesigner;
    if (!designer) return;

    // 清空现有动作
    designer.actions = [];

    // 添加示例动作 - 展示并行执行逻辑
    const exampleActions = [
        {
            id: Date.now() + 1,
            structure: 'RF',
            forwardAngle: 30,
            bendAngle: 20,
            speed: 2,
            delayTime: 0,
            startTime: 0,  // RF从0开始
            duration: 300
        },
        {
            id: Date.now() + 2,
            structure: 'LF',
            forwardAngle: -30,
            bendAngle: 15,
            speed: 2,
            delayTime: 0,
            startTime: 0,  // LF也从0开始（并行执行）
            duration: 300
        },
        {
            id: Date.now() + 3,
            structure: 'RF',
            forwardAngle: -30,
            bendAngle: -20,
            speed: 2,
            delayTime: 0,
            startTime: 300,  // RF的第二个动作，在第一个动作结束后开始
            duration: 300
        },
        {
            id: Date.now() + 4,
            structure: 'RE',
            forwardAngle: 20,
            bendAngle: 10,
            speed: 2,
            delayTime: 500,  // 有延迟时间
            startTime: 500,  // 延迟500ms后开始
            duration: 200
        }
    ];

    designer.actions = exampleActions;
    designer.updateTimeline();
    designer.updateStatus();
    designer.updateCodeOutput();
}

// 在控制台中可以调用 loadExample() 来加载示例数据
window.loadExample = loadExample;