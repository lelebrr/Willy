/* ============================================
   WILLY 2.1 - Hub de Segurança Ofensiva
   JavaScript Principal
   ============================================ */

// ============================================
// UTILITÁRIOS
// ============================================

const $ = (s) => document.querySelector(s);
const $$ = (s) => document.querySelectorAll(s);

const IS_DEV = (window.location.host === "127.0.0.1:8080");

// Criptografia simples para credenciais
const Crypto = {
  encode: (str) => btoa(encodeURIComponent(str).split('').reverse().join('')),
  decode: (str) => { try { return decodeURIComponent(atob(str).split('').reverse().join('')); } catch { return null; } },
  hash: (str) => {
    let hash = 0;
    for (let i = 0; i < str.length; i++) {
      const char = str.charCodeAt(i);
      hash = ((hash << 5) - hash) + char;
      hash = hash & hash;
    }
    return Math.abs(hash).toString(16);
  }
};

// ============================================
// SISTEMA DE AUTENTICAÇÃO
// ============================================

const Auth = {
  STORAGE_KEY: 'willy_auth',
  SESSION_KEY: 'willy_session',
  
  async verificarPrimeiroAcesso() {
    const credenciais = localStorage.getItem(this.STORAGE_KEY);
    return !credenciais;
  },
  
  async criarConta(usuario, senha) {
    const credenciais = {
      usuario: Crypto.encode(usuario),
      senha: Crypto.hash(senha + usuario),
      criado: new Date().toISOString()
    };
    
    localStorage.setItem(this.STORAGE_KEY, JSON.stringify(credenciais));
    this.criarSessao(usuario);
    return true;
  },
  
  async login(usuario, senha) {
    const credenciais = JSON.parse(localStorage.getItem(this.STORAGE_KEY));
    if (!credenciais) return false;
    
    const usuarioDecodificado = Crypto.decode(credenciais.usuario);
    const senhaHash = Crypto.hash(senha + usuario);
    
    if (usuario === usuarioDecodificado && senhaHash === credenciais.senha) {
      this.criarSessao(usuario);
      return true;
    }
    return false;
  },
  
  criarSessao(usuario) {
    const sessao = {
      usuario,
      token: Crypto.encode(Date.now() + usuario),
      expira: Date.now() + (24 * 60 * 60 * 1000) // 24 horas
    };
    sessionStorage.setItem(this.SESSION_KEY, JSON.stringify(sessao));
  },
  
  verificarSessao() {
    const sessao = JSON.parse(sessionStorage.getItem(this.SESSION_KEY));
    if (!sessao) return false;
    if (Date.now() > sessao.expira) {
      this.logout();
      return false;
    }
    return true;
  },
  
  logout() {
    sessionStorage.removeItem(this.SESSION_KEY);
  },
  
  async atualizarCredenciais(novoUsuario, novaSenha) {
    const credenciais = JSON.parse(localStorage.getItem(this.STORAGE_KEY));
    if (!credenciais) return false;
    
    if (novoUsuario) {
      credenciais.usuario = Crypto.encode(novoUsuario);
    }
    if (novaSenha) {
      const usuario = novoUsuario || Crypto.decode(credenciais.usuario);
      credenciais.senha = Crypto.hash(novaSenha + usuario);
    }
    credenciais.atualizado = new Date().toISOString();
    
    localStorage.setItem(this.STORAGE_KEY, JSON.stringify(credenciais));
    return true;
  }
};

// ============================================
// SISTEMA DE NAVEGAÇÃO
// ============================================

const Navigation = {
  paginaAtual: 'dashboard',
  
  init() {
    // Click nos itens do menu
    $$('.nav-item').forEach(item => {
      item.addEventListener('click', (e) => {
        e.preventDefault();
        const pagina = item.dataset.page;
        this.irPara(pagina);
      });
    });
    
    // Toggle sidebar
    $('#toggle-sidebar')?.addEventListener('click', () => {
      $('#sidebar')?.classList.toggle('collapsed');
    });
    
    // Toggle tema
    $('#toggle-theme')?.addEventListener('click', () => {
      document.body.classList.toggle('light-theme');
      const icon = $('#toggle-theme i');
      if (icon) {
        icon.className = document.body.classList.contains('light-theme') ? 'fas fa-sun' : 'fas fa-moon';
      }
    });
    
    // Atalhos de teclado
    document.addEventListener('keydown', (e) => this.atalhosTeclado(e));
  },
  
  irPara(pagina) {
    $$('.nav-item').forEach(item => {
      item.classList.toggle('active', item.dataset.page === pagina);
    });
    
    $$('.page').forEach(p => {
      p.classList.toggle('active', p.id === `page-${pagina}`);
    });
    
    const titulos = {
      dashboard: 'Painel',
      navigator: 'Navegador',
      modules: 'Módulos',
      files: 'Arquivos',
      logs: 'Logs',
      terminal: 'Terminal',
      capturas: 'Capturas',
      scripts: 'Scripts',
      configuracoes: 'Configurações'
    };
    
    $('#page-title').textContent = titulos[pagina] || 'Painel';
    this.paginaAtual = pagina;
    
    // Inicializar página
    if (pagina === 'dashboard') Painel.init();
    if (pagina === 'navigator') Navegador.init();
    if (pagina === 'files') Arquivos.init();
    if (pagina === 'terminal') Terminal.init();
    if (pagina === 'logs') Logs.init();
    if (pagina === 'scripts') Scripts.init();
    if (pagina === 'capturas') Capturas.init();
  },
  
  atalhosTeclado(e) {
    // Atalhos do navegador
    if (this.paginaAtual === 'navigator') {
      const mapa = {
        'ArrowUp': 'Up',
        'ArrowDown': 'Down',
        'ArrowLeft': 'Prev',
        'ArrowRight': 'Next',
        'Enter': 'Sel',
        'Backspace': 'Esc',
        'PageUp': 'NextPage',
        'PageDown': 'PrevPage'
      };
      
      if (e.key === 'r' || e.key === 'R') {
        e.preventDefault();
        Navegador.recarregarTela();
        return;
      }
      
      if (mapa[e.key]) {
        e.preventDefault();
        Navegador.enviarComando(mapa[e.key]);
      }
    }
    
    // Escape fecha dialogs
    if (e.key === 'Escape') {
      Dialog.fechar();
    }
  }
};

// ============================================
// SISTEMA DE NOTIFICAÇÕES
// ============================================

const Toast = {
  mostrar(mensagem, tipo = 'info', duracao = 4000) {
    const container = $('#toast-container');
    const toast = document.createElement('div');
    toast.className = `toast ${tipo}`;
    
    const icones = {
      success: 'fa-check-circle',
      error: 'fa-exclamation-circle',
      warning: 'fa-exclamation-triangle',
      info: 'fa-info-circle'
    };
    
    toast.innerHTML = `
      <i class="fas ${icones[tipo]}"></i>
      <span>${mensagem}</span>
      <button onclick="this.parentElement.remove()" style="background:none;border:none;color:inherit;margin-left:auto;cursor:pointer;">
        <i class="fas fa-times"></i>
      </button>
    `;
    
    container.appendChild(toast);
    
    setTimeout(() => {
      toast.style.animation = 'toastIn 0.3s ease reverse';
      setTimeout(() => toast.remove(), 300);
    }, duracao);
  }
};

// ============================================
// SISTEMA DE LOADING
// ============================================

const Loading = {
  mostrar(mensagem = 'Carregando...') {
    $('#loading-overlay')?.classList.remove('hidden');
    $('#loading-text').textContent = mensagem;
  },
  
  esconder() {
    $('#loading-overlay')?.classList.add('hidden');
  }
};

// ============================================
// SISTEMA DE DIALOGS
// ============================================

const Dialog = {
  atual: null,
  
  mostrar(id) {
    this.fechar();
    $('#dialog-overlay')?.classList.remove('hidden');
    const dialog = $(`#${id}`);
    if (dialog) {
      dialog.classList.remove('hidden');
      this.atual = dialog;
    }
  },
  
  fechar() {
    $('#dialog-overlay')?.classList.add('hidden');
    $$('.dialog').forEach(d => d.classList.add('hidden'));
    this.atual = null;
  },
  
  confirmar(titulo, mensagem, callback) {
    $('#confirm-title').textContent = titulo;
    $('#confirm-message').textContent = mensagem;
    
    const btn = $('#confirm-btn');
    btn.onclick = () => {
      callback();
      this.fechar();
    };
    
    this.mostrar('dialog-confirm');
  }
};

// ============================================
// REQUISIÇÕES HTTP
// ============================================

async function requisicaoGet(url, dados = null) {
  try {
    let urlReal = IS_DEV ? "/willy" + url : url;
    if (dados) {
      const params = new URLSearchParams(dados);
      urlReal += "?" + params.toString();
    }
    
    const response = await fetch(urlReal);
    
    if (response.status === 401) {
      Auth.logout();
      window.location.reload();
      throw new Error('Não autorizado');
    }
    
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    return await response.text();
  } catch (error) {
    console.error('Erro na requisição:', error);
    throw error;
  }
}

async function requisicaoPost(url, dados = {}) {
  try {
    const fd = new FormData();
    for (let key in dados) {
      if (dados.hasOwnProperty(key)) fd.append(key, dados[key]);
    }
    
    let urlReal = IS_DEV ? "/willy" + url : url;
    const response = await fetch(urlReal, { method: 'POST', body: fd });
    
    if (response.status === 401) {
      Auth.logout();
      window.location.reload();
      throw new Error('Não autorizado');
    }
    
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    return await response.text();
  } catch (error) {
    console.error('Erro na requisição:', error);
    throw error;
  }
}

async function executarComando(cmd) {
  Loading.mostrar('Executando comando...');
  try {
    await requisicaoPost("/cm", { cmnd: cmd });
    Toast.mostrar(`Comando executado: ${cmd}`, 'success');
  } catch (error) {
    Toast.mostrar(`Erro ao executar comando: ${error.message}`, 'error');
  } finally {
    Loading.hide();
  }
}

// ============================================
// PAINEL / DASHBOARD
// ============================================

const Painel = {
  graficoMemoria: null,
  graficoRSSI: null,
  dadosMemoria: [],
  dadosRSSI: [],
  intervaloAtualizacao: null,
  
  init() {
    this.carregarDados();
    this.iniciarGraficos();
    
    // Atualizar a cada 5 segundos
    if (this.intervaloAtualizacao) clearInterval(this.intervaloAtualizacao);
    this.intervaloAtualizacao = setInterval(() => this.carregarDados(), 5000);
  },
  
  async carregarDados() {
    try {
      const infoStr = await requisicaoGet("/systeminfo");
      const info = JSON.parse(infoStr);
      
      // Atualizar stats
      this.atualizarStats(info);
      
      // Atualizar gráficos
      this.atualizarGraficos(info);
      
      // Atualizar sidebar
      $('#sidebar-ram').textContent = info.heapFree || '--';
      $('#sidebar-sd').textContent = info.SD?.total || '--';
      
    } catch (error) {
      console.error('Erro ao carregar dados:', error);
    }
  },
  
  atualizarStats(info) {
    // RAM
    const ramLivre = info.heapFree || 0;
    const ramTotal = info.heapSize || 300000;
    $('#stat-ram').textContent = Math.round(ramLivre / 1024) + ' KB livre';
    const ramPct = Math.round((1 - ramLivre / ramTotal) * 100);
    $('#ram-bar').style.width = ramPct + '%';
    
    // SD
    if (info.SD) {
      $('#stat-sd').textContent = info.SD.used + ' / ' + info.SD.total;
      $('#sd-size').textContent = info.SD.total;
    }
    
    // Flash
    if (info.LittleFS) {
      $('#stat-flash').textContent = info.LittleFS.used + ' / ' + info.LittleFS.total;
      $('#flash-size').textContent = info.LittleFS.total;
    }
    
    // WiFi
    if (info.wifiConnected) {
      $('#stat-wifi').textContent = 'Conectado';
      $('#wifi-ip').textContent = info.wifiIP || '--';
      $('#mod-wifi-status').textContent = 'Ativo';
      $('#mod-wifi-status').classList.add('active');
    } else {
      $('#stat-wifi').textContent = 'Desconectado';
      $('#wifi-ip').textContent = '--';
    }
    
    // Info do sistema
    $('#info-chip').textContent = info.chipModel || 'ESP32';
    $('#info-cpu').textContent = (info.cpuFreq || 240) + ' MHz';
    $('#info-screen').textContent = (info.screenWidth || '--') + ' x ' + (info.screenHeight || '--');
    $('#info-version').textContent = info.BRUCE_VERSION || '2.1';
  },
  
  iniciarGraficos() {
    // Gráfico de Memória
    const ctxMem = $('#memory-chart');
    if (!ctxMem) return;
    
    this.graficoMemoria = new Chart(ctxMem, {
      type: 'line',
      data: {
        labels: [],
        datasets: [{
          label: 'Memória Livre (KB)',
          data: [],
          borderColor: '#00f0ff',
          backgroundColor: 'rgba(0, 240, 255, 0.1)',
          fill: true,
          tension: 0.4,
          pointRadius: 0
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        scales: {
          x: { display: false },
          y: {
            min: 0,
            grid: { color: 'rgba(255,255,255,0.05)' },
            ticks: { color: '#505070' }
          }
        },
        plugins: { legend: { display: false } }
      }
    });
    
    // Gráfico RSSI
    const ctxRSSI = $('#rssi-chart');
    if (!ctxRSSI) return;
    
    this.graficoRSSI = new Chart(ctxRSSI, {
      type: 'line',
      data: {
        labels: [],
        datasets: [{
          label: 'RSSI (dBm)',
          data: [],
          borderColor: '#00ff88',
          backgroundColor: 'rgba(0, 255, 136, 0.1)',
          fill: true,
          tension: 0.4,
          pointRadius: 0
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        scales: {
          x: { display: false },
          y: {
            min: -100,
            max: 0,
            grid: { color: 'rgba(255,255,255,0.05)' },
            ticks: { color: '#505070' }
          }
        },
        plugins: { legend: { display: false } }
      }
    });
  },
  
  atualizarGraficos(info) {
    const agora = new Date();
    const hora = agora.toLocaleTimeString();
    
    // Memória
    this.dadosMemoria.push({
      hora,
      valor: Math.round((info.heapFree || 0) / 1024)
    });
    if (this.dadosMemoria.length > 30) this.dadosMemoria.shift();
    
    if (this.graficoMemoria) {
      this.graficoMemoria.data.labels = this.dadosMemoria.map(d => d.hora);
      this.graficoMemoria.data.datasets[0].data = this.dadosMemoria.map(d => d.valor);
      this.graficoMemoria.update('none');
    }
    
    // RSSI
    const rssi = info.wifiRSSI || -70;
    this.dadosRSSI.push({ hora, valor: parseInt(rssi) });
    if (this.dadosRSSI.length > 30) this.dadosRSSI.shift();
    
    if (this.graficoRSSI) {
      this.graficoRSSI.data.labels = this.dadosRSSI.map(d => d.hora);
      this.graficoRSSI.data.datasets[0].data = this.dadosRSSI.map(d => d.valor);
      this.graficoRSSI.update('none');
    }
  }
};

// ============================================
// NAVEGADOR REMOTO
// ============================================

const Navegador = {
  carregando: false,
  intervaloAutoRefresh: null,
  
  init() {
    // Auto-refresh
    const selectAutoRefresh = $('#auto-refresh');
    if (selectAutoRefresh) {
      selectAutoRefresh.addEventListener('change', () => {
        const intervalo = parseInt(selectAutoRefresh.value);
        if (this.intervaloAutoRefresh) clearInterval(this.intervaloAutoRefresh);
        if (intervalo > 0) {
          this.intervaloAutoRefresh = setInterval(() => this.recarregarTela(), intervalo);
        }
      });
    }
    
    // Botões de navegação
    $$('.nav-btn').forEach(btn => {
      btn.addEventListener('click', () => {
        const dir = btn.dataset.dir;
        if (dir) this.enviarComando(dir);
      });
    });
    
    this.recarregarTela();
  },
  
  async enviarComando(direcao) {
    if (this.carregando) return;
    this.carregando = true;
    
    try {
      this.mostrarLoading();
      let cmd = 'nav ' + direcao.toLowerCase();
      if (direcao === 'Menu') cmd = 'nav sel 500';
      await requisicaoPost("/cm", { cmnd: cmd });
      await this.recarregarTela();
    } catch (error) {
      Toast.mostrar('Erro na navegação: ' + error.message, 'error');
    } finally {
      this.carregando = false;
    }
  },
  
  async recarregarTela() {
    try {
      this.mostrarLoading();
      const response = await fetch((IS_DEV ? "/willy" : "") + "/getscreen");
      const arrayBuffer = await response.arrayBuffer();
      const screenData = new Uint8Array(arrayBuffer);
      await this.renderizarTela(screenData);
    } catch (error) {
      console.error('Erro ao carregar tela:', error);
    } finally {
      this.esconderLoading();
    }
  },
  
  mostrarLoading() {
    $('#screen-loading')?.classList.remove('hidden');
  },
  
  esconderLoading() {
    $('#screen-loading')?.classList.add('hidden');
  },
  
  async renderizarTela(data) {
    const canvas = $('#device-screen');
    if (!canvas) return;
    const ctx = canvas.getContext("2d");
    
    const color565toCSS = (color565) => {
      const r = ((color565 >> 11) & 0x1F) * 255 / 31;
      const g = ((color565 >> 5) & 0x3F) * 255 / 63;
      const b = (color565 & 0x1F) * 255 / 31;
      return `rgb(${r},${g},${b})`;
    };
    
    let offset = 0;
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    
    while (offset < data.length) {
      if (data[offset] !== 0xAA) break;
      
      offset++;
      const size = data[offset++];
      const fn = data[offset++];
      
      // Implementação simplificada do parser TFT
      // Suporta comandos básicos de desenho
      
      switch (fn) {
        case 99: // Resize canvas
          canvas.width = (data[offset] << 8) | data[offset + 1];
          canvas.height = (data[offset + 2] << 8) | data[offset + 3];
          break;
        case 0: // Fill screen
          const cor = (data[offset] << 8) | data[offset + 1];
          ctx.fillStyle = color565toCSS(cor);
          ctx.fillRect(0, 0, canvas.width, canvas.height);
          break;
        case 2: // Fill rect
          const x = (data[offset] << 8) | data[offset + 1];
          const y = (data[offset + 2] << 8) | data[offset + 3];
          const w = (data[offset + 4] << 8) | data[offset + 5];
          const h = (data[offset + 6] << 8) | data[offset + 7];
          const c = (data[offset + 8] << 8) | data[offset + 9];
          ctx.fillStyle = color565toCSS(c);
          ctx.fillRect(x, y, w, h);
          break;
      }
      
      offset += size - 3;
    }
    
    this.esconderLoading();
  }
};

// ============================================
// ARQUIVOS
// ============================================

const Arquivos = {
  driveAtual: 'SD',
  caminhoAtual: '/',
  sdDisponivel: true,
  
  EXECUTAVEIS: {
    ir: "ir tx_from_file",
    sub: "subghz tx_from_file",
    js: "js run_from_file",
    bjs: "js run_from_file",
    txt: "badusb run_from_file",
    mp3: "play",
    wav: "play"
  },
  
  init() {
    // Botões de drive
    $$('.drive-btn').forEach(btn => {
      btn.addEventListener('click', () => {
        const drive = btn.dataset.drive;
        if (drive && drive !== this.driveAtual) {
          this.driveAtual = drive;
          this.caminhoAtual = '/';
          $$('.drive-btn').forEach(b => b.classList.remove('active'));
          btn.classList.add('active');
          this.carregar();
        }
      });
    });
    
    this.carregar();
  },
  
  async carregar() {
    try {
      const lista = await requisicaoGet("/listfiles", { fs: this.driveAtual, folder: this.caminhoAtual });
      this.renderizar(lista);
      this.atualizarBreadcrumb();
    } catch (error) {
      Toast.mostrar('Erro ao carregar arquivos', 'error');
    }
  },
  
  renderizar(lista) {
    const grid = $('#files-grid');
    if (!grid) return;
    
    grid.innerHTML = '';
    
    const linhas = lista.split("\n").sort((a, b) => {
      const [aTipo, ...aResto] = a.split(':');
      const [bTipo, ...bResto] = b.split(':');
      if (aTipo !== bTipo) return bTipo.localeCompare(aTipo);
      return aResto.join(':').toLowerCase().localeCompare(bResto.join(':').toLowerCase());
    });
    
    linhas.forEach((linha) => {
      const [tipo, nome, tamanho] = linha.split(":");
      if (tamanho === undefined) return;
      
      const caminhoCompleto = ((this.caminhoAtual.endsWith("/") ? this.caminhoAtual : this.caminhoAtual + "/") + nome).replace(/\/\//g, "/");
      
      const item = document.createElement('div');
      
      if (tipo === "pa") {
        item.className = 'file-item folder';
        item.innerHTML = `
          <div class="file-icon folder"><i class="fas fa-folder"></i></div>
          <div class="file-info">
            <div class="file-name">..</div>
            <div class="file-size">Pasta anterior</div>
          </div>
        `;
        item.onclick = () => {
          this.caminhoAtual = this.caminhoAtual.substring(0, this.caminhoAtual.lastIndexOf('/')) || '/';
          this.carregar();
        };
      } else if (tipo === "Fi") {
        const ext = nome.split('.').pop().toLowerCase();
        const executavel = this.EXECUTAVEIS[ext];
        item.className = 'file-item file' + (executavel ? ' executable' : '');
        item.innerHTML = `
          <div class="file-icon ${executavel ? 'executable' : 'file'}">
            <i class="fas ${executavel ? 'fa-play' : 'fa-file'}"></i>
          </div>
          <div class="file-info">
            <div class="file-name">${nome}</div>
            <div class="file-size">${tamanho}</div>
          </div>
          <div class="file-actions">
            ${executavel ? `<button onclick="event.stopPropagation(); Arquivos.executar('${caminhoCompleto}')" title="Executar"><i class="fas fa-play"></i></button>` : ''}
            <button onclick="event.stopPropagation(); Arquivos.editar('${caminhoCompleto}')" title="Editar"><i class="fas fa-edit"></i></button>
            <button onclick="event.stopPropagation(); Arquivos.baixar('${caminhoCompleto}', '${nome}')" title="Baixar"><i class="fas fa-download"></i></button>
            <button onclick="event.stopPropagation(); Arquivos.excluir('${caminhoCompleto}')" title="Excluir"><i class="fas fa-trash"></i></button>
          </div>
        `;
      } else if (tipo === "Fo") {
        item.className = 'file-item folder';
        item.innerHTML = `
          <div class="file-icon folder"><i class="fas fa-folder"></i></div>
          <div class="file-info">
            <div class="file-name">${nome}</div>
            <div class="file-size">Pasta</div>
          </div>
          <div class="file-actions">
            <button onclick="event.stopPropagation(); Arquivos.renomear('${caminhoCompleto}')" title="Renomear"><i class="fas fa-edit"></i></button>
            <button onclick="event.stopPropagation(); Arquivos.excluir('${caminhoCompleto}')" title="Excluir"><i class="fas fa-trash"></i></button>
          </div>
        `;
        item.onclick = () => {
          this.caminhoAtual = caminhoCompleto;
          this.carregar();
        };
      }
      
      grid.appendChild(item);
    });
  },
  
  atualizarBreadcrumb() {
    const breadcrumb = $('#breadcrumb-path');
    if (!breadcrumb) return;
    
    breadcrumb.innerHTML = '';
    
    const partes = this.caminhoAtual.split('/').filter(p => p);
    
    const home = document.createElement('span');
    home.className = 'path-item' + (partes.length === 0 ? ' active' : '');
    home.innerHTML = '<i class="fas fa-home"></i>';
    home.onclick = () => { this.caminhoAtual = '/'; this.carregar(); };
    breadcrumb.appendChild(home);
    
    let caminho = '';
    partes.forEach((parte, i) => {
      caminho += '/' + parte;
      const item = document.createElement('span');
      item.className = 'path-item' + (i === partes.length - 1 ? ' active' : '');
      item.textContent = parte;
      item.onclick = () => { this.caminhoAtual = caminho; this.carregar(); };
      breadcrumb.appendChild(item);
    });
  },
  
  async executar(caminho) {
    const ext = caminho.split('.').pop().toLowerCase();
    const cmd = this.EXECUTAVEIS[ext];
    if (cmd) {
      await executarComando(cmd + ' "' + caminho + '"');
    }
  },
  
  async editar(caminho) {
    try {
      Loading.mostrar('Carregando arquivo...');
      const conteudo = await requisicaoGet(`/file?fs=${this.driveAtual}&name=${encodeURIComponent(caminho)}&action=edit`);
      
      $('#editor-filename').textContent = caminho;
      $('#editor-content').value = conteudo;
      this.atualizarNumerosLinha();
      
      const ext = caminho.split('.').pop().toLowerCase();
      const executavel = this.EXECUTAVEIS[ext];
      $('#btn-executar').classList.toggle('hidden', !executavel);
      
      Dialog.mostrar('dialog-editor');
    } catch (error) {
      Toast.mostrar('Erro ao carregar arquivo', 'error');
    } finally {
      Loading.esconder();
    }
  },
  
  atualizarNumerosLinha() {
    const textarea = $('#editor-content');
    const lineNumbers = $('#line-numbers');
    if (!textarea || !lineNumbers) return;
    
    const linhas = textarea.value.split('\n');
    lineNumbers.textContent = linhas.map((_, i) => i + 1).join('\n');
  },
  
  async salvar() {
    const caminho = $('#editor-filename').textContent;
    const conteudo = $('#editor-content').value;
    
    try {
      Loading.mostrar('Salvando...');
      await requisicaoPost("/edit", { fs: this.driveAtual, name: caminho, content: conteudo });
      Toast.mostrar('Arquivo salvo!', 'success');
    } catch (error) {
      Toast.mostrar('Erro ao salvar arquivo', 'error');
    } finally {
      Loading.esconder();
    }
  },
  
  async executarEditor() {
    await this.salvar();
    const caminho = $('#editor-filename').textContent;
    await this.executar(caminho);
  },
  
  baixar(caminho, nome) {
    const url = (IS_DEV ? "/willy" : "") + `/file?fs=${this.driveAtual}&name=${encodeURIComponent(caminho)}&action=download`;
    const a = document.createElement('a');
    a.href = url;
    a.download = nome;
    a.click();
  },
  
  excluir(caminho) {
    Dialog.confirmar('Excluir arquivo', `Tem certeza que deseja excluir ${caminho}?`, async () => {
      try {
        Loading.mostrar('Excluindo...');
        await requisicaoGet("/file", { fs: this.driveAtual, action: 'delete', name: caminho });
        Toast.mostrar('Arquivo excluído!', 'success');
        this.carregar();
      } catch (error) {
        Toast.mostrar('Erro ao excluir arquivo', 'error');
      } finally {
        Loading.esconder();
      }
    });
  },
  
  renomear(caminho) {
    const novoNome = prompt('Novo nome:', caminho.split('/').pop());
    if (!novoNome) return;
    
    requisicaoPost("/rename", { fs: this.driveAtual, filePath: caminho, fileName: novoNome })
      .then(() => { Toast.mostrar('Renomeado!', 'success'); this.carregar(); })
      .catch(() => Toast.mostrar('Erro ao renomear', 'error'));
  }
};

// ============================================
// TERMINAL
// ============================================

const Terminal = {
  historico: [],
  indiceHistorico: -1,
  
  init() {
    const input = $('#terminal-input');
    if (input) {
      input.addEventListener('keydown', (e) => {
        if (e.key === 'Enter') {
          this.enviar();
        } else if (e.key === 'ArrowUp') {
          if (this.indiceHistorico < this.historico.length - 1) {
            this.indiceHistorico++;
            input.value = this.historico[this.historico.length - 1 - this.indiceHistorico];
          }
        } else if (e.key === 'ArrowDown') {
          if (this.indiceHistorico > 0) {
            this.indiceHistorico--;
            input.value = this.historico[this.historico.length - 1 - this.indiceHistorico];
          } else if (this.indiceHistorico === 0) {
            this.indiceHistorico = -1;
            input.value = '';
          }
        }
      });
    }
  },
  
  async enviar() {
    const input = $('#terminal-input');
    const output = $('#terminal-output');
    if (!input || !output) return;
    
    const cmd = input.value.trim();
    if (!cmd) return;
    
    this.historico.push(cmd);
    this.indiceHistorico = -1;
    
    const linha = document.createElement('div');
    linha.className = 'terminal-line command';
    linha.textContent = 'willy> ' + cmd;
    output.appendChild(linha);
    
    input.value = '';
    
    try {
      await requisicaoPost("/cm", { cmnd: cmd });
      const resultado = document.createElement('div');
      resultado.className = 'terminal-line output';
      resultado.textContent = 'Comando executado.';
      output.appendChild(resultado);
    } catch (error) {
      const erro = document.createElement('div');
      erro.className = 'terminal-line error';
      erro.textContent = 'Erro: ' + error.message;
      output.appendChild(erro);
    }
    
    output.scrollTop = output.scrollHeight;
  },
  
  limpar() {
    const output = $('#terminal-output');
    if (output) {
      output.innerHTML = `
        <div class="terminal-welcome">
          <pre class="ascii-art">
██╗    ██╗██╗██╗     ██╗     ███████╗██████╗ 
██║    ██║██║██║     ██║     ██╔════╝██╔══██╗
██║ █╗ ██║██║██║     ██║     █████╗  ██████╔╝
██║███╗██║██║██║     ██║     ██╔══╝  ██╔══██╗
╚███╔███╔╝██║███████╗███████╗███████╗██║  ██║
 ╚══╝╚══╝ ╚═╝╚══════╝╚══════╝╚══════╝╚═╝  ╚═╝
          </pre>
          <p>Digite <code>ajuda</code> para ver os comandos disponíveis.</p>
        </div>
      `;
    }
  }
};

// ============================================
// LOGS
// ============================================

const Logs = {
  dados: [],
  
  init() {
    // Filtros
    $('#log-filter')?.addEventListener('change', () => this.filtrar());
    $('#log-search')?.addEventListener('input', () => this.filtrar());
  },
  
  adicionar(modulo, mensagem, nivel = 'info') {
    const entrada = {
      tempo: new Date().toLocaleTimeString(),
      modulo,
      mensagem,
      nivel
    };
    this.dados.push(entrada);
    this.renderizar();
  },
  
  filtrar() {
    this.renderizar();
  },
  
  renderizar() {
    const container = $('#logs-container');
    if (!container) return;
    
    const filtroModulo = $('#log-filter')?.value || 'all';
    const busca = $('#log-search')?.value?.toLowerCase() || '';
    
    const filtrados = this.dados.filter(l => {
      if (filtroModulo !== 'all' && l.modulo.toLowerCase() !== filtroModulo) return false;
      if (busca && !l.mensagem.toLowerCase().includes(busca)) return false;
      return true;
    });
    
    container.innerHTML = filtrados.map(l => `
      <div class="log-entry">
        <span class="log-time">${l.tempo}</span>
        <span class="log-module">${l.modulo}</span>
        <span class="log-level-${l.nivel}">[${l.nivel.toUpperCase()}]</span>
        <span class="log-message">${l.mensagem}</span>
      </div>
    `).join('');
  },
  
  limpar() {
    this.dados = [];
    this.renderizar();
  },
  
  exportar() {
    const texto = this.dados.map(l => `${l.tempo} [${l.modulo}] ${l.nivel.toUpperCase()}: ${l.mensagem}`).join('\n');
    const blob = new Blob([texto], { type: 'text/plain' });
    const a = document.createElement('a');
    a.href = URL.createObjectURL(blob);
    a.download = 'willy_logs_' + new Date().toISOString().slice(0, 10) + '.txt';
    a.click();
  }
};

// ============================================
// SCRIPTS
// ============================================

const Scripts = {
  init() {
    this.carregar();
  },
  
  async carregar() {
    try {
      const lista = await requisicaoGet("/listfiles", { fs: 'SD', folder: '/interpreter' });
      this.renderizar(lista);
    } catch (error) {
      console.error('Erro ao carregar scripts:', error);
    }
  },
  
  renderizar(lista) {
    const grid = $('#scripts-grid');
    if (!grid) return;
    
    grid.innerHTML = '';
    
    lista.split("\n").forEach((linha) => {
      const [tipo, nome, tamanho] = linha.split(":");
      if (tipo === "Fi" && (nome.endsWith('.js') || nome.endsWith('.bjs'))) {
        const item = document.createElement('div');
        item.className = 'script-item';
        item.innerHTML = `
          <div class="script-icon"><i class="fas fa-code"></i></div>
          <div class="script-name">${nome}</div>
          <div class="script-meta">${tamanho}</div>
        `;
        item.onclick = () => Arquivos.editar('/interpreter/' + nome);
        grid.appendChild(item);
      }
    });
  }
};

// ============================================
// CAPTURAS
// ============================================

const Capturas = {
  init() {
    this.carregar();
  },
  
  async carregar() {
    // Carregar capturas de diferentes pastas
    const tipos = ['pcap', 'infrared', 'subghz', 'nfc', 'gps'];
    // Implementar listagem de capturas
  }
};

// ============================================
// INICIALIZAÇÃO
// ============================================

async function inicializar() {
  // Verificar se é primeiro acesso
  const primeiroAcesso = await Auth.verificarPrimeiroAcesso();
  
  if (primeiroAcesso) {
    // Mostrar tela de setup
    $('#setup-screen')?.classList.remove('hidden');
    setupEventListeners();
  } else if (Auth.verificarSessao()) {
    // Já logado, mostrar app
    mostrarAplicacao();
  } else {
    // Mostrar tela de login
    $('#login-screen')?.classList.remove('hidden');
    loginEventListeners();
  }
  
  // Atualizar relógio
  setInterval(() => {
    $('#header-time').textContent = new Date().toLocaleTimeString();
  }, 1000);
}

function setupEventListeners() {
  const form = $('#setup-form');
  if (form) {
    form.addEventListener('submit', async (e) => {
      e.preventDefault();
      
      const usuario = $('#setup-username').value;
      const senha = $('#setup-password').value;
      const confirmar = $('#setup-password-confirm').value;
      
      if (senha !== confirmar) {
        Toast.mostrar('As senhas não coincidem', 'error');
        return;
      }
      
      if (senha.length < 6) {
        Toast.mostrar('A senha deve ter pelo menos 6 caracteres', 'error');
        return;
      }
      
      await Auth.criarConta(usuario, senha);
      Toast.mostrar('Conta criada com sucesso!', 'success');
      mostrarAplicacao();
    });
    
    // Força da senha
    $('#setup-password')?.addEventListener('input', (e) => {
      const senha = e.target.value;
      const barra = $('#password-strength .strength-bar');
      const texto = $('#password-strength .strength-text');
      
      barra.classList.remove('weak', 'medium', 'strong');
      
      if (senha.length < 6) {
        barra.classList.add('weak');
        texto.textContent = 'Fraca';
      } else if (senha.length < 10 || !/[A-Z]/.test(senha) || !/[0-9]/.test(senha)) {
        barra.classList.add('medium');
        texto.textContent = 'Média';
      } else {
        barra.classList.add('strong');
        texto.textContent = 'Forte';
      }
    });
  }
}

function loginEventListeners() {
  const form = $('#login-form');
  if (form) {
    form.addEventListener('submit', async (e) => {
      e.preventDefault();
      
      const usuario = $('#login-username').value;
      const senha = $('#login-password').value;
      
      const sucesso = await Auth.login(usuario, senha);
      
      if (sucesso) {
        Toast.mostrar('Login realizado!', 'success');
        mostrarAplicacao();
      } else {
        const erro = $('#login-error');
        erro.classList.remove('hidden');
        erro.querySelector('span').textContent = 'Usuário ou senha incorretos';
      }
    });
  }
}

function mostrarAplicacao() {
  $('#setup-screen')?.classList.add('hidden');
  $('#login-screen')?.classList.add('hidden');
  $('#app-container')?.classList.remove('hidden');
  
  Navigation.init();
  Painel.init();
}

// Funções globais
function togglePasswordVisibility(id) {
  const input = $('#' + id);
  const icon = input?.nextElementSibling?.querySelector('i') || input?.parentElement.querySelector('button i');
  if (input) {
    if (input.type === 'password') {
      input.type = 'text';
      if (icon) { icon.classList.remove('fa-eye'); icon.classList.add('fa-eye-slash'); }
    } else {
      input.type = 'password';
      if (icon) { icon.classList.remove('fa-eye-slash'); icon.classList.add('fa-eye'); }
    }
  }
}

function sair() {
  Auth.logout();
  window.location.reload();
}

function irPara(pagina) {
  Navigation.irPara(pagina);
}

function fecharDialog() {
  Dialog.fechar();
}

function salvarArquivo() {
  Arquivos.salvar();
}

function executarArquivo() {
  Arquivos.executarEditor();
}

function recarregarArquivos() {
  Arquivos.carregar();
}

function uploadArquivos() {
  const input = document.createElement('input');
  input.type = 'file';
  input.multiple = true;
  input.onchange = async (e) => {
    const arquivos = e.target.files;
    for (let arquivo of arquivos) {
      await Arquivos.upload(arquivo);
    }
    Arquivos.carregar();
  };
  input.click();
}

function criarPasta() {
  const nome = prompt('Nome da pasta:');
  if (!nome) return;
  const caminho = Arquivos.caminhoAtual.replace(/\/+$/, '') + '/' + nome;
  requisicaoGet("/file", { fs: Arquivos.driveAtual, action: 'create', name: caminho })
    .then(() => { Toast.mostrar('Pasta criada!', 'success'); Arquivos.carregar(); })
    .catch(() => Toast.mostrar('Erro ao criar pasta', 'error'));
}

function criarArquivo() {
  const nome = prompt('Nome do arquivo:');
  if (!nome) return;
  const caminho = Arquivos.caminhoAtual.replace(/\/+$/, '') + '/' + nome;
  requisicaoGet("/file", { fs: Arquivos.driveAtual, action: 'createfile', name: caminho })
    .then(() => { Toast.mostrar('Arquivo criado!', 'success'); Arquivos.carregar(); })
    .catch(() => Toast.mostrar('Erro ao criar arquivo', 'error'));
}

function criarNovoScript() {
  const nome = prompt('Nome do script (ex: script.js):');
  if (!nome) return;
  $('#editor-filename').textContent = '/interpreter/' + nome;
  $('#editor-content').value = '// Script Willy\n// ...\n\n';
  Arquivos.atualizarNumerosLinha();
  $('#btn-executar').classList.remove('hidden');
  Dialog.mostrar('dialog-editor');
}

function novoScript() {
  criarNovoScript();
}

function recarregarTela() {
  Navegador.recarregarTela();
}

function enviarComando() {
  Terminal.enviar();
}

function limparTerminal() {
  Terminal.limpar();
}

function limparLogs() {
  Logs.limpar();
}

function exportarLogs() {
  Logs.exportar();
}

function confirmarReboot() {
  Dialog.confirmar('Reiniciar dispositivo', 'Tem certeza que deseja reiniciar o dispositivo?', async () => {
    await executarComando('reboot');
    setTimeout(() => location.reload(), 3000);
  });
}

function resetConfig() {
  Dialog.confirmar('Restaurar padrões', 'Tem certeza que deseja restaurar as configurações padrão?', async () => {
    await executarComando('factory_reset');
  });
}

async function salvarCredenciais() {
  const usuario = $('#config-username').value;
  const senha = $('#config-password').value;
  const confirmar = $('#config-password-confirm').value;
  
  if (senha && senha !== confirmar) {
    Toast.mostrar('As senhas não coincidem', 'error');
    return;
  }
  
  const sucesso = await Auth.atualizarCredenciais(usuario || null, senha || null);
  if (sucesso) {
    Toast.mostrar('Credenciais atualizadas!', 'success');
    $('#config-password').value = '';
    $('#config-password-confirm').value = '';
  } else {
    Toast.mostrar('Erro ao atualizar credenciais', 'error');
  }
}

// Brilho slider
$('#config-brightness')?.addEventListener('input', (e) => {
  const valor = e.target.value;
  $('#brightness-value').textContent = valor;
  executarComando(`bright ${valor}`);
});

// Rotação
$('#config-rotation')?.addEventListener('change', (e) => {
  executarComando(`rotation ${e.target.value}`);
});

// Editor line numbers
$('#editor-content')?.addEventListener('input', () => Arquivos.atualizarNumerosLinha());
$('#editor-content')?.addEventListener('scroll', function() {
  const lineNumbers = $('#line-numbers');
  if (lineNumbers) lineNumbers.scrollTop = this.scrollTop;
});

// Dialog overlay click
$('#dialog-overlay')?.addEventListener('click', (e) => {
  if (e.target === $('#dialog-overlay')) Dialog.fechar();
});

// Drag & drop upload
document.addEventListener('DOMContentLoaded', () => {
  const uploadArea = $('#upload-area');
  
  document.addEventListener('dragenter', () => uploadArea?.classList.remove('hidden'));
  
  uploadArea?.addEventListener('dragleave', (e) => {
    if (e.target === uploadArea) uploadArea.classList.add('hidden');
  });
  
  uploadArea?.addEventListener('dragover', (e) => e.preventDefault());
  
  uploadArea?.addEventListener('drop', async (e) => {
    e.preventDefault();
    uploadArea.classList.add('hidden');
    const arquivos = e.dataTransfer.files;
    for (let arquivo of arquivos) {
      await Arquivos.upload(arquivo);
    }
    Arquivos.carregar();
  });
  
  // Inicializar
  inicializar();
});

// Console message
console.log('%c WILLY 2.1 ', 'background: #00f0ff; color: #0a0a12; font-size: 20px; font-weight: bold;');
console.log('%c Hub de Segurança Ofensiva ', 'background: #12121f; color: #00f0ff; font-size: 14px;');
console.log('Use com responsabilidade!');

// ============================================
// CONFIGURAÇÕES DO LOGO
// ============================================

const LogoConfig = {
  config: {
    velocidade: 1,
    somAtivado: true,
    tipoSom: 0,
    corCustomizada: false,
    cor: '#00f0ff'
  },
  
  init() {
    // Carregar configurações
    this.carregar();
    
    // Toggle cor customizada
    $('#logo-cor-custom')?.addEventListener('change', (e) => {
      const corGroup = $('#logo-cor-group');
      if (corGroup) {
        corGroup.style.display = e.target.checked ? 'block' : 'none';
      }
    });
    
    // Preview da cor
    $('#logo-cor')?.addEventListener('input', (e) => {
      const preview = $('#logo-cor-preview');
      if (preview) {
        preview.textContent = e.target.value.toUpperCase();
      }
    });
  },
  
  carregar() {
    // Tentar carregar do localStorage
    const salvo = localStorage.getItem('willy_logo_config');
    if (salvo) {
      try {
        this.config = JSON.parse(salvo);
        this.atualizarUI();
      } catch (e) {
        console.error('Erro ao carregar config do logo:', e);
      }
    }
  },
  
  salvar() {
    localStorage.setItem('willy_logo_config', JSON.stringify(this.config));
  },
  
  atualizarUI() {
    const v = $('#logo-velocidade');
    if (v) v.value = this.config.velocidade;
    
    const s = $('#logo-som');
    if (s) s.checked = this.config.somAtivado;
    
    const ts = $('#logo-tipo-som');
    if (ts) ts.value = this.config.tipoSom;
    
    const cc = $('#logo-cor-custom');
    if (cc) {
      cc.checked = this.config.corCustomizada;
      const corGroup = $('#logo-cor-group');
      if (corGroup) {
        corGroup.style.display = this.config.corCustomizada ? 'block' : 'none';
      }
    }
    
    const c = $('#logo-cor');
    if (c) {
      c.value = this.config.cor;
      const preview = $('#logo-cor-preview');
      if (preview) {
        preview.textContent = this.config.cor.toUpperCase();
      }
    }
  },
  
  lerDaUI() {
    const v = $('#logo-velocidade');
    if (v) this.config.velocidade = parseInt(v.value);
    
    const s = $('#logo-som');
    if (s) this.config.somAtivado = s.checked;
    
    const ts = $('#logo-tipo-som');
    if (ts) this.config.tipoSom = parseInt(ts.value);
    
    const cc = $('#logo-cor-custom');
    if (cc) this.config.corCustomizada = cc.checked;
    
    const c = $('#logo-cor');
    if (c) this.config.cor = c.value;
  }
};

async function salvarConfigLogo() {
  LogoConfig.lerDaUI();
  LogoConfig.salvar();
  
  // Enviar para o dispositivo
  try {
    Loading.mostrar('Salvando configurações...');
    
    const corHex = LogoConfig.config.cor.replace('#', '');
    const cmd = `logo config ${LogoConfig.config.velocidade} ${LogoConfig.config.somAtivado ? 1 : 0} ${LogoConfig.config.tipoSom} ${LogoConfig.config.corCustomizada ? 1 : 0} ${corHex}`;
    
    await requisicaoPost("/cm", { cmnd: cmd });
    Toast.mostrar('Configurações do logo salvas!', 'success');
  } catch (error) {
    Toast.mostrar('Erro ao salvar configurações do logo', 'error');
  } finally {
    Loading.esconder();
  }
}

async function testarSplash() {
  try {
    Loading.mostrar('Testando splash screen...');
    await requisicaoPost("/cm", { cmnd: "logo test" });
    Toast.mostrar('Splash screen exibida no dispositivo!', 'info');
  } catch (error) {
    Toast.mostrar('Erro ao testar splash screen', 'error');
  } finally {
    Loading.esconder();
  }
}

// Inicializar configurações do logo quando a página de configurações for aberta
const originalIrPara = Navigation.irPara.bind(Navigation);
Navigation.irPara = function(pagina) {
  originalIrPara(pagina);
  if (pagina === 'configuracoes') {
    LogoConfig.init();
  }
};
