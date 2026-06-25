(function () {
  'use strict';

  var Module = null;

  // ── Helpers ──────────────────────────────────────────────────────────

  function call(fn, args, types) {
    var raw = Module.ccall(fn, 'string', types || [], args || []);
    return JSON.parse(raw);
  }

  function todayStr() {
    var d = new Date();
    var dd = String(d.getDate()).padStart(2, '0');
    var mm = String(d.getMonth() + 1).padStart(2, '0');
    var yyyy = d.getFullYear();
    return dd + '/' + mm + '/' + yyyy;
  }

  function normalizeDate(input) {
    if (!input) return input;
    var s = input.replace(/\D/g, '');
    if (s.length === 8) {
      return s.substring(0, 2) + '/' + s.substring(2, 4) + '/' + s.substring(4, 8);
    }
    return input;
  }

  // ── SVG Icons (reusable) ────────────────────────────────────────────

  var ICONS = {
    calendar: '<svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="4" width="18" height="18" rx="2"/><line x1="16" y1="2" x2="16" y2="6"/><line x1="8" y1="2" x2="8" y2="6"/><line x1="3" y1="10" x2="21" y2="10"/></svg>',
    user: '<svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2"/><circle cx="12" cy="7" r="4"/></svg>',
    clipboard: '<svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><rect x="8" y="2" width="8" height="4" rx="1"/><path d="M16 4h2a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2H6a2 2 0 0 1-2-2V6a2 2 0 0 1 2-2h2"/></svg>',
    check: '<svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"/><path d="M9 12l2 2 4-4"/></svg>',
    alertTriangle: '<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"/><line x1="12" y1="8" x2="12" y2="12"/><line x1="12" y1="16" x2="12.01" y2="16"/></svg>'
  };

  // ── Empty State ─────────────────────────────────────────────────────

  function emptyStateHtml(icon, message, actionLabel, actionScreen) {
    var html = '<div class="empty-state">';
    html += '<div class="empty-state-icon">' + (ICONS[icon] || ICONS.calendar) + '</div>';
    html += '<div class="empty-state-text">' + message + '</div>';
    if (actionLabel && actionScreen) {
      html += '<button class="btn btn-secondary empty-state-action" data-nav="' + actionScreen + '">' + actionLabel + '</button>';
    }
    html += '</div>';
    return html;
  }

  function bindEmptyStateActions(container) {
    var btns = (container || document).querySelectorAll('.empty-state-action[data-nav]');
    for (var i = 0; i < btns.length; i++) {
      btns[i].addEventListener('click', function () {
        navigate(this.getAttribute('data-nav'));
      });
    }
  }

  // ── Form Validation Helpers ─────────────────────────────────────────

  function showFieldError(fieldId, msg) {
    clearFieldError(fieldId);
    var field = document.getElementById(fieldId);
    if (!field) return;
    var group = field.closest('.form-group');
    if (group) group.classList.add('has-error');
    var errEl = document.createElement('div');
    errEl.className = 'form-error-msg';
    errEl.innerHTML = ICONS.alertTriangle + ' ' + msg;
    field.parentNode.appendChild(errEl);
  }

  function clearFieldError(fieldId) {
    var field = document.getElementById(fieldId);
    if (!field) return;
    var group = field.closest('.form-group');
    if (!group) return;
    group.classList.remove('has-error');
    var existing = group.querySelector('.form-error-msg');
    if (existing) existing.remove();
  }

  function clearAllFieldErrors() {
    var errors = document.querySelectorAll('.form-error-msg');
    for (var i = 0; i < errors.length; i++) errors[i].remove();
    var groups = document.querySelectorAll('.form-group.has-error');
    for (var j = 0; j < groups.length; j++) groups[j].classList.remove('has-error');
  }

  // ── Toast ────────────────────────────────────────────────────────────

  function toast(msg, type) {
    var el = document.createElement('div');
    el.className = 'toast ' + (type === 'error' ? 'toast-error' : 'toast-success');
    el.textContent = msg;
    document.body.appendChild(el);
    setTimeout(function () { el.remove(); }, 3500);
  }

  // ── Navigation ───────────────────────────────────────────────────────

  function navigate(screenId) {
    var screens = document.querySelectorAll('.screen');
    for (var i = 0; i < screens.length; i++) {
      screens[i].classList.remove('active');
    }
    var target = document.getElementById('screen-' + screenId);
    if (target) target.classList.add('active');

    var btns = document.querySelectorAll('.nav-btn');
    for (var j = 0; j < btns.length; j++) {
      btns[j].classList.remove('active');
      if (btns[j].getAttribute('data-screen') === screenId) {
        btns[j].classList.add('active');
      }
    }

    // Close sidebar on mobile
    var sidebar = document.getElementById('sidebar');
    var overlay = document.getElementById('sidebar-overlay');
    if (sidebar) sidebar.classList.remove('open');
    if (overlay) overlay.classList.remove('open');

    clearAllFieldErrors();

    // Load screen data
    var loaders = {
      'dashboard': loadDashboard,
      'pac-listar': loadPacientes,
      'med-listar': loadMedicos,
      'agendar': initAgendamento
    };
    if (loaders[screenId]) loaders[screenId]();
  }

  // ── Dashboard ────────────────────────────────────────────────────────

  function loadDashboard() {
    var c = call('bridge_contadores', [todayStr()], ['string']);
    var cardsEl = document.getElementById('dash-cards');
    cardsEl.innerHTML =
      '<div class="metric-card"><div class="label">Pacientes</div><div class="value primary">' + c.pacientes + '</div></div>' +
      '<div class="metric-card"><div class="label">Medicos</div><div class="value success">' + c.medicos + '</div></div>' +
      '<div class="metric-card"><div class="label">Agendamentos Ativos</div><div class="value warning">' + c.agendamentos + '</div></div>' +
      '<div class="metric-card"><div class="label">Consultas Hoje</div><div class="value danger">' + c.consultas_hoje + '</div></div>';

    var agenda = call('bridge_agenda_dia', [todayStr()], ['string']);
    var hojeEl = document.getElementById('dash-hoje');
    if (!agenda || agenda.length === 0) {
      hojeEl.innerHTML = emptyStateHtml('calendar', 'Nenhuma consulta agendada para hoje.', 'Criar agendamento', 'agendar');
      bindEmptyStateActions(hojeEl);
      return;
    }
    var rows = '';
    for (var i = 0; i < agenda.length; i++) {
      var med = agenda[i];
      if (!med.slots) continue;
      for (var s = 0; s < med.slots.length; s++) {
        var sl = med.slots[s];
        if (sl.ocupado) {
          rows += '<tr><td>' + sl.hora + '</td><td>' + med.nome + '</td><td>' + (med.especialidade || '') + '</td><td>' + (sl.paciente || '') + '</td><td>' + (sl.queixa || '') + '</td></tr>';
        }
      }
    }
    if (!rows) {
      hojeEl.innerHTML = emptyStateHtml('calendar', 'Nenhuma consulta agendada para hoje.', 'Criar agendamento', 'agendar');
      bindEmptyStateActions(hojeEl);
      return;
    }
    hojeEl.innerHTML =
      '<table><thead><tr><th>Hora</th><th>Medico</th><th>Especialidade</th><th>Paciente</th><th>Queixa</th></tr></thead><tbody>' +
      rows + '</tbody></table>';
  }

  // ── Pacientes: Cadastrar ─────────────────────────────────────────────

  function setupPacCadastrar() {
    document.getElementById('pac-cadastrar-btn').addEventListener('click', function () {
      clearAllFieldErrors();
      var cpf = document.getElementById('pac-cpf').value.trim();
      var nome = document.getElementById('pac-nome').value.trim();
      var nasc = document.getElementById('pac-nasc').value.trim();
      var tel = document.getElementById('pac-tel').value.trim();
      var hasError = false;
      if (!cpf) { showFieldError('pac-cpf', 'CPF e obrigatorio'); hasError = true; }
      if (!nome) { showFieldError('pac-nome', 'Nome e obrigatorio'); hasError = true; }
      if (!nasc) { showFieldError('pac-nasc', 'Data de nascimento e obrigatoria'); hasError = true; }
      if (!tel) { showFieldError('pac-tel', 'Telefone e obrigatorio'); hasError = true; }
      if (hasError) return;
      var r = call('bridge_paciente_cadastrar', [cpf, nome, nasc, tel], ['string', 'string', 'string', 'string']);
      if (r.ok) {
        toast('Paciente cadastrado com sucesso!');
        document.getElementById('pac-cpf').value = '';
        document.getElementById('pac-nome').value = '';
        document.getElementById('pac-nasc').value = '';
        document.getElementById('pac-tel').value = '';
        clearAllFieldErrors();
      } else {
        toast(r.erro || 'Erro ao cadastrar.', 'error');
      }
    });
  }

  // ── Pacientes: Listar ────────────────────────────────────────────────

  function loadPacientes() {
    var lista = call('bridge_paciente_listar', [], []);
    var el = document.getElementById('pac-lista');
    if (!lista || lista.length === 0) {
      el.innerHTML = emptyStateHtml('user', 'Nenhum paciente cadastrado ainda.', 'Cadastrar paciente', 'pac-cadastrar');
      bindEmptyStateActions(el);
      return;
    }
    var rows = '';
    for (var i = 0; i < lista.length; i++) {
      var p = lista[i];
      rows += '<tr><td>' + p.cpf + '</td><td>' + p.nome + '</td><td>' + p.data_nascimento + '</td><td>' + p.telefone + '</td></tr>';
    }
    el.innerHTML =
      '<table><thead><tr><th>CPF</th><th>Nome</th><th>Nascimento</th><th>Telefone</th></tr></thead><tbody>' +
      rows + '</tbody></table>';
  }

  // ── Historico Paciente ───────────────────────────────────────────────

  function setupHistorico() {
    document.getElementById('hist-buscar-btn').addEventListener('click', function () {
      clearAllFieldErrors();
      var cpf = document.getElementById('hist-cpf').value.trim();
      if (!cpf) { showFieldError('hist-cpf', 'Informe o CPF'); return; }
      var pac = call('bridge_paciente_buscar_cpf', [cpf], ['string']);
      if (!pac.encontrado) {
        toast('Paciente nao encontrado.', 'error');
        document.getElementById('hist-resultado').innerHTML = '';
        return;
      }
      var r = call('bridge_agendamento_listar_paciente', [cpf, 0], ['string', 'number']);
      var el = document.getElementById('hist-resultado');
      var html = '<div class="summary-box"><strong>Paciente:</strong> ' + pac.nome + '<br><strong>CPF:</strong> ' + pac.cpf + '</div>';
      if (!r.ok || r.total === 0) {
        html += emptyStateHtml('calendar', 'Nenhum agendamento encontrado para este paciente.');
        el.innerHTML = html;
        return;
      }
      var rows = '';
      for (var i = 0; i < r.agendamentos.length; i++) {
        var a = r.agendamentos[i];
        var status = a.cancelado ? '<span class="badge badge-danger">Cancelado</span>' : '<span class="badge badge-success">Ativo</span>';
        rows += '<tr><td>' + a.protocolo + '</td><td>' + a.data + '</td><td>' + a.hora + '</td><td>' + a.medico + '</td><td>' + a.especialidade + '</td><td>' + (a.queixa || '') + '</td><td>' + status + '</td></tr>';
      }
      html += '<table><thead><tr><th>Protocolo</th><th>Data</th><th>Hora</th><th>Medico</th><th>Especialidade</th><th>Queixa</th><th>Status</th></tr></thead><tbody>' + rows + '</tbody></table>';
      el.innerHTML = html;
    });
  }

  // ── Medicos: Cadastrar ───────────────────────────────────────────────

  function loadEspecialidades() {
    var esp = call('bridge_especialidades_listar', [], []);
    var sel = document.getElementById('med-esp');
    sel.innerHTML = '';
    for (var i = 0; i < esp.length; i++) {
      var opt = document.createElement('option');
      opt.value = i;
      opt.textContent = esp[i];
      sel.appendChild(opt);
    }
  }

  function setupMedCadastrar() {
    document.getElementById('med-cadastrar-btn').addEventListener('click', function () {
      clearAllFieldErrors();
      var crm = document.getElementById('med-crm').value.trim();
      var nome = document.getElementById('med-nome').value.trim();
      var espIdx = parseInt(document.getElementById('med-esp').value, 10);
      var hasError = false;
      if (!crm) { showFieldError('med-crm', 'CRM e obrigatorio'); hasError = true; }
      if (!nome) { showFieldError('med-nome', 'Nome e obrigatorio'); hasError = true; }
      if (hasError) return;
      var r = call('bridge_medico_cadastrar', [crm, nome, espIdx], ['string', 'string', 'number']);
      if (r.ok) {
        toast('Medico cadastrado com sucesso!');
        document.getElementById('med-crm').value = '';
        document.getElementById('med-nome').value = '';
        clearAllFieldErrors();
      } else {
        toast(r.erro || 'Erro ao cadastrar.', 'error');
      }
    });
  }

  // ── Medicos: Listar ──────────────────────────────────────────────────

  function loadMedicos() {
    var lista = call('bridge_medico_listar', [], []);
    var el = document.getElementById('med-lista');
    if (!lista || lista.length === 0) {
      el.innerHTML = emptyStateHtml('clipboard', 'Nenhum medico cadastrado ainda.', 'Cadastrar medico', 'med-cadastrar');
      bindEmptyStateActions(el);
      return;
    }
    var rows = '';
    for (var i = 0; i < lista.length; i++) {
      var m = lista[i];
      rows += '<tr><td>' + m.crm + '</td><td>' + m.nome + '</td><td>' + m.especialidade + '</td></tr>';
    }
    el.innerHTML =
      '<table><thead><tr><th>CRM</th><th>Nome</th><th>Especialidade</th></tr></thead><tbody>' +
      rows + '</tbody></table>';
  }

  // ── Medicos: Disponibilidade ─────────────────────────────────────────

  var dispCrmAtual = '';

  function renderDisp(crm, disp) {
    var dias = ['Domingo', 'Segunda', 'Terca', 'Quarta', 'Quinta', 'Sexta', 'Sabado'];
    var html = '<div class="disp-grid">';
    html += '<div class="dg-header"></div><div class="dg-header">Manha</div><div class="dg-header">Tarde</div>';
    for (var d = 0; d < 7; d++) {
      html += '<div class="dg-day">' + dias[d] + '</div>';
      for (var t = 0; t < 2; t++) {
        var on = disp[d][t];
        html += '<div class="disp-cell ' + (on ? 'on' : 'off') + '" data-dia="' + d + '" data-turno="' + t + '">' + (on ? 'Disponivel' : '-') + '</div>';
      }
    }
    html += '</div>';
    return html;
  }

  function setupDisp() {
    document.getElementById('disp-buscar-btn').addEventListener('click', function () {
      clearAllFieldErrors();
      var crm = document.getElementById('disp-crm').value.trim();
      if (!crm) { showFieldError('disp-crm', 'Informe o CRM'); return; }
      var med = call('bridge_medico_buscar_crm', [crm], ['string']);
      if (!med.encontrado) {
        toast('Medico nao encontrado.', 'error');
        document.getElementById('disp-resultado').innerHTML = '';
        return;
      }
      dispCrmAtual = crm;
      var r = call('bridge_medico_disponibilidade_obter', [crm], ['string']);
      if (!r.ok) {
        toast('Erro ao obter disponibilidade.', 'error');
        return;
      }
      var el = document.getElementById('disp-resultado');
      el.innerHTML = '<div class="summary-box"><strong>' + med.nome + '</strong> - ' + med.especialidade + '</div>' + renderDisp(crm, r.disp);
      bindDispCells();
    });
  }

  function bindDispCells() {
    var cells = document.querySelectorAll('#disp-resultado .disp-cell');
    for (var i = 0; i < cells.length; i++) {
      cells[i].addEventListener('click', function () {
        var dia = parseInt(this.getAttribute('data-dia'), 10);
        var turno = parseInt(this.getAttribute('data-turno'), 10);
        var r = call('bridge_medico_disponibilidade_alternar', [dispCrmAtual, dia, turno], ['string', 'number', 'number']);
        if (r.ok) {
          var med = call('bridge_medico_buscar_crm', [dispCrmAtual], ['string']);
          var el = document.getElementById('disp-resultado');
          el.innerHTML = '<div class="summary-box"><strong>' + med.nome + '</strong> - ' + med.especialidade + '</div>' + renderDisp(dispCrmAtual, r.disp);
          bindDispCells();
        }
      });
    }
  }

  // ── Agendar (multi-step) ─────────────────────────────────────────────

  var agState = {
    step: 0,
    cpf: '',
    pacNome: '',
    espIdx: -1,
    espNome: '',
    crm: '',
    medNome: '',
    data: '',
    slotIdx: -1,
    slotHora: '',
    queixa: ''
  };

  var AG_STEPS = 5;
  var AG_LABELS = ['Paciente', 'Especialidade', 'Horario', 'Queixa', 'Confirmacao'];

  function renderSteps() {
    var el = document.getElementById('ag-steps');
    var html = '';
    for (var i = 0; i < AG_STEPS; i++) {
      var cls = 'step-item';
      if (i < agState.step) cls += ' done';
      else if (i === agState.step) cls += ' active';
      var content = (i < agState.step) ? '✓' : String(i + 1);
      html += '<div class="' + cls + '">';
      html += '<div class="step-number">' + content + '</div>';
      html += '<div class="step-label">' + AG_LABELS[i] + '</div>';
      html += '</div>';
      if (i < AG_STEPS - 1) {
        html += '<div class="step-line' + (i < agState.step ? ' done' : '') + '"></div>';
      }
    }
    el.innerHTML = html;
  }

  function initAgendamento() {
    agState.step = 0;
    agState.cpf = '';
    agState.pacNome = '';
    agState.espIdx = -1;
    agState.espNome = '';
    agState.crm = '';
    agState.medNome = '';
    agState.data = '';
    agState.slotIdx = -1;
    agState.slotHora = '';
    agState.queixa = '';
    renderAgStep();
  }

  function renderAgStep() {
    renderSteps();
    var el = document.getElementById('ag-content');

    if (agState.step === 0) {
      el.innerHTML =
        '<h3>Identificar Paciente</h3>' +
        '<div style="max-width:400px;">' +
        '<div class="form-group"><label>CPF</label><input type="text" id="ag-cpf" placeholder="00000000000" maxlength="11"></div>' +
        '<div class="btn-row"><button class="btn btn-primary" id="ag-cpf-btn">Buscar</button></div>' +
        '</div>';
      document.getElementById('ag-cpf-btn').addEventListener('click', function () {
        clearAllFieldErrors();
        var cpf = document.getElementById('ag-cpf').value.trim();
        if (!cpf) { showFieldError('ag-cpf', 'Informe o CPF'); return; }
        var r = call('bridge_paciente_buscar_cpf', [cpf], ['string']);
        if (!r.encontrado) { toast('Paciente nao encontrado.', 'error'); return; }
        agState.cpf = r.cpf;
        agState.pacNome = r.nome;
        agState.step = 1;
        renderAgStep();
      });

    } else if (agState.step === 1) {
      var esp = call('bridge_especialidades_listar', [], []);
      var html = '<h3>Selecionar Especialidade</h3>' +
        '<div class="summary-box"><strong>Paciente:</strong> ' + agState.pacNome + ' (' + agState.cpf + ')</div>' +
        '<div class="esp-cards">';
      for (var i = 0; i < esp.length; i++) {
        html += '<div class="esp-card" data-idx="' + i + '" data-nome="' + esp[i] + '">' + esp[i] + '</div>';
      }
      html += '</div><div class="btn-row"><button class="btn btn-secondary" id="ag-back-1">Voltar</button></div>';
      el.innerHTML = html;

      var cards = document.querySelectorAll('#ag-content .esp-card');
      for (var j = 0; j < cards.length; j++) {
        cards[j].addEventListener('click', function () {
          agState.espIdx = parseInt(this.getAttribute('data-idx'), 10);
          agState.espNome = this.getAttribute('data-nome');
          agState.step = 2;
          renderAgStep();
        });
      }
      document.getElementById('ag-back-1').addEventListener('click', function () {
        agState.step = 0;
        renderAgStep();
      });

    } else if (agState.step === 2) {
      var medicos = call('bridge_medico_buscar_especialidade', [agState.espIdx], ['number']);
      var html2 = '<h3>Selecionar Data e Horario</h3>' +
        '<div class="summary-box"><strong>Paciente:</strong> ' + agState.pacNome + '<br><strong>Especialidade:</strong> ' + agState.espNome + '</div>';

      if (!medicos || medicos.length === 0) {
        html2 += '<div class="alert alert-danger">Nenhum medico encontrado para esta especialidade.</div>';
        html2 += '<div class="btn-row"><button class="btn btn-secondary" id="ag-back-2">Voltar</button></div>';
        el.innerHTML = html2;
        document.getElementById('ag-back-2').addEventListener('click', function () { agState.step = 1; renderAgStep(); });
        return;
      }

      var medicoDisps = [];
      for (var md = 0; md < medicos.length; md++) {
        var dr = call('bridge_medico_disponibilidade_obter', [medicos[md].crm], ['string']);
        medicoDisps.push(dr.ok ? dr.disp : null);
      }

      var hoje = new Date();
      var diasCal = [];
      var diasNome = ['Dom', 'Seg', 'Ter', 'Qua', 'Qui', 'Sex', 'Sab'];
      for (var di = 0; di < 14; di++) {
        var dt = new Date(hoje); dt.setDate(hoje.getDate() + di);
        var ddS = String(dt.getDate()).padStart(2, '0');
        var mmS = String(dt.getMonth() + 1).padStart(2, '0');
        diasCal.push({ full: ddS + '/' + mmS + '/' + dt.getFullYear(), short: ddS + '/' + mmS, name: diasNome[dt.getDay()], dow: dt.getDay() });
      }

      var temAlgum = false;
      for (var dc = 0; dc < diasCal.length; dc++) {
        var diaInfo = diasCal[dc];
        var diaHtml = '';
        for (var mi = 0; mi < medicos.length; mi++) {
          var mDisp = medicoDisps[mi];
          if (!mDisp || (!mDisp[diaInfo.dow][0] && !mDisp[diaInfo.dow][1])) continue;
          var gr = call('bridge_grade_obter_ou_criar', [medicos[mi].crm, diaInfo.full], ['string', 'string']);
          if (!gr.ok || !gr.slots) continue;
          var livres = [];
          for (var si = 0; si < gr.slots.length; si++) { if (!gr.slots[si].ocupado) livres.push(gr.slots[si]); }
          if (livres.length === 0) continue;
          diaHtml += '<div class="date-doctor"><div class="date-doctor-name">' + medicos[mi].nome + ' <span style="color:var(--color-text-secondary);font-size:12px;">CRM: ' + medicos[mi].crm + '</span></div><div class="slots-grid">';
          for (var li = 0; li < livres.length; li++) {
            diaHtml += '<div class="slot-card" data-crm="' + medicos[mi].crm + '" data-nome="' + medicos[mi].nome + '" data-date="' + diaInfo.full + '" data-idx="' + livres[li].idx + '" data-hora="' + livres[li].hora + '"><div class="slot-time">' + livres[li].hora + '</div></div>';
          }
          diaHtml += '</div></div>';
        }
        if (diaHtml) {
          temAlgum = true;
          html2 += '<div class="date-card"><div class="date-card-header">' + diaInfo.short + ' <span>' + diaInfo.name + '</span></div>' + diaHtml + '</div>';
        }
      }

      if (!temAlgum) {
        html2 += '<div class="alert alert-info">Nenhum horario disponivel nos proximos 14 dias para ' + agState.espNome + '.</div>';
      }

      html2 += '<div id="ag-cal-btn-area"></div>';
      html2 += '<div class="btn-row"><button class="btn btn-secondary" id="ag-back-2">Voltar</button></div>';
      el.innerHTML = html2;

      var calSlots = el.querySelectorAll('.slot-card');
      for (var cs = 0; cs < calSlots.length; cs++) {
        calSlots[cs].addEventListener('click', function () {
          var allS = el.querySelectorAll('.slot-card');
          for (var x = 0; x < allS.length; x++) allS[x].classList.remove('selected');
          this.classList.add('selected');
          agState.crm = this.getAttribute('data-crm');
          agState.medNome = this.getAttribute('data-nome');
          agState.data = this.getAttribute('data-date');
          agState.slotIdx = parseInt(this.getAttribute('data-idx'), 10);
          agState.slotHora = this.getAttribute('data-hora');
          var area = document.getElementById('ag-cal-btn-area');
          if (!document.getElementById('ag-cal-next')) {
            area.innerHTML = '<div class="btn-row"><button class="btn btn-primary" id="ag-cal-next">Continuar</button></div>';
            document.getElementById('ag-cal-next').addEventListener('click', function () { agState.step = 3; renderAgStep(); });
          }
        });
      }

      document.getElementById('ag-back-2').addEventListener('click', function () { agState.step = 1; renderAgStep(); });

    } else if (agState.step === 3) {
      el.innerHTML =
        '<h3>Informar Queixa</h3>' +
        '<div class="summary-box"><strong>Paciente:</strong> ' + agState.pacNome + '<br><strong>Medico:</strong> ' + agState.medNome + '<br><strong>Data:</strong> ' + agState.data + '<br><strong>Horario:</strong> ' + agState.slotHora + '</div>' +
        '<div style="max-width:480px;margin-top:16px;">' +
        '<div class="form-group"><label>Queixa / Motivo da Consulta</label><textarea id="ag-queixa" placeholder="Descreva a queixa do paciente"></textarea></div>' +
        '<div class="btn-row"><button class="btn btn-secondary" id="ag-back-3">Voltar</button><button class="btn btn-primary" id="ag-queixa-btn">Continuar</button></div>' +
        '</div>';

      document.getElementById('ag-queixa-btn').addEventListener('click', function () {
        clearAllFieldErrors();
        agState.queixa = document.getElementById('ag-queixa').value.trim();
        if (!agState.queixa) { showFieldError('ag-queixa', 'Informe a queixa do paciente'); return; }
        agState.step = 4;
        renderAgStep();
      });
      document.getElementById('ag-back-3').addEventListener('click', function () {
        agState.step = 2;
        renderAgStep();
      });

    } else if (agState.step === 4) {
      el.innerHTML =
        '<h3>Confirmar Agendamento</h3>' +
        '<div class="summary-box">' +
        '<strong>Paciente:</strong> ' + agState.pacNome + ' (' + agState.cpf + ')<br>' +
        '<strong>Medico:</strong> ' + agState.medNome + ' (CRM: ' + agState.crm + ')<br>' +
        '<strong>Especialidade:</strong> ' + agState.espNome + '<br>' +
        '<strong>Data:</strong> ' + agState.data + '<br>' +
        '<strong>Horario:</strong> ' + agState.slotHora + '<br>' +
        '<strong>Queixa:</strong> ' + agState.queixa +
        '</div>' +
        '<div class="btn-row"><button class="btn btn-secondary" id="ag-back-5">Voltar</button><button class="btn btn-primary" id="ag-confirm-btn">Confirmar Agendamento</button></div>';

      document.getElementById('ag-confirm-btn').addEventListener('click', function () {
        var r = call('bridge_agendamento_criar', [agState.cpf, agState.crm, agState.data, agState.slotIdx, agState.queixa], ['string', 'string', 'string', 'number', 'string']);
        if (r.ok) {
          toast('Agendamento criado! Protocolo: ' + r.protocolo);
          el.innerHTML =
            '<div class="confirmation-card">' +
            '<div class="confirmation-icon">' + ICONS.check + '</div>' +
            '<div class="confirmation-title">Agendamento Confirmado</div>' +
            '<div class="confirmation-protocol">' +
            '<div class="confirmation-protocol-label">Protocolo</div>' +
            '<div class="confirmation-protocol-number">' + r.protocolo + '</div>' +
            '</div>' +
            '<div class="confirmation-details">' +
            '<strong>Paciente:</strong> ' + agState.pacNome + '<br>' +
            '<strong>Medico:</strong> ' + agState.medNome + '<br>' +
            '<strong>Especialidade:</strong> ' + agState.espNome + '<br>' +
            '<strong>Data:</strong> ' + agState.data + ' - ' + agState.slotHora + '<br>' +
            '<strong>Queixa:</strong> ' + agState.queixa +
            '</div>' +
            '<div class="btn-row" style="justify-content:center;"><button class="btn btn-primary" id="ag-novo-btn">Novo Agendamento</button></div>' +
            '</div>';
          document.getElementById('ag-steps').innerHTML = '';
          document.getElementById('ag-novo-btn').addEventListener('click', initAgendamento);
        } else {
          toast(r.erro || 'Erro ao agendar.', 'error');
        }
      });
      document.getElementById('ag-back-5').addEventListener('click', function () {
        agState.step = 3;
        renderAgStep();
      });
    }
  }

  function renderSlots(slots) {
    var el = document.getElementById('ag-slots');
    if (!slots || slots.length === 0) {
      el.innerHTML = '<div class="alert alert-danger">Nenhum horario disponivel nesta data.</div>';
      return;
    }

    var manha = [];
    var tarde = [];
    for (var i = 0; i < slots.length; i++) {
      var hora = parseInt(slots[i].hora.substring(0, 2), 10);
      if (hora < 12) {
        manha.push(slots[i]);
      } else {
        tarde.push(slots[i]);
      }
    }

    var html = '';
    if (manha.length > 0) {
      html += '<div class="turno-label">Manha</div><div class="slots-grid">';
      for (var m = 0; m < manha.length; m++) {
        html += slotCardHtml(manha[m]);
      }
      html += '</div>';
    }
    if (tarde.length > 0) {
      html += '<div class="turno-label">Tarde</div><div class="slots-grid">';
      for (var t = 0; t < tarde.length; t++) {
        html += slotCardHtml(tarde[t]);
      }
      html += '</div>';
    }
    el.innerHTML = html;

    var slotCards = el.querySelectorAll('.slot-card:not(.occupied)');
    for (var j = 0; j < slotCards.length; j++) {
      slotCards[j].addEventListener('click', function () {
        var all = el.querySelectorAll('.slot-card');
        for (var x = 0; x < all.length; x++) all[x].classList.remove('selected');
        this.classList.add('selected');
        agState.slotIdx = parseInt(this.getAttribute('data-idx'), 10);
        agState.slotHora = this.getAttribute('data-hora');

        if (!document.getElementById('ag-slot-next-btn')) {
          var btnRow = document.createElement('div');
          btnRow.className = 'btn-row';
          btnRow.id = 'ag-slot-next-row';
          btnRow.innerHTML = '<button class="btn btn-primary" id="ag-slot-next-btn">Continuar</button>';
          el.appendChild(btnRow);
          document.getElementById('ag-slot-next-btn').addEventListener('click', function () {
            if (agState.slotIdx < 0) { toast('Selecione um horario.', 'error'); return; }
            agState.step = 4;
            renderAgStep();
          });
        }
      });
    }
  }

  function slotCardHtml(s) {
    var cls = 'slot-card';
    if (s.ocupado) cls += ' occupied';
    var status = s.ocupado ? 'Ocupado' : 'Livre';
    var patient = s.ocupado && s.paciente ? '<div class="slot-patient">' + s.paciente + '</div>' : '';
    return '<div class="' + cls + '" data-idx="' + s.idx + '" data-hora="' + s.hora + '">' +
      '<div class="slot-time">' + s.hora + '</div>' +
      '<div class="slot-status">' + status + '</div>' +
      patient + '</div>';
  }

  // ── Cancelar ─────────────────────────────────────────────────────────

  function setupCancelar() {
    document.getElementById('cancel-buscar-btn').addEventListener('click', function () {
      clearAllFieldErrors();
      var proto = document.getElementById('cancel-proto').value.trim();
      if (!proto) { showFieldError('cancel-proto', 'Informe o protocolo'); return; }
      var protoNum = parseInt(proto, 10);
      var r = call('bridge_agendamento_buscar_protocolo', [protoNum], ['number']);
      var el = document.getElementById('cancel-resultado');
      if (!r.encontrado) {
        toast('Protocolo nao encontrado.', 'error');
        el.innerHTML = '';
        return;
      }
      if (r.cancelado) {
        el.innerHTML = '<div class="alert alert-danger">Este agendamento ja foi cancelado.</div>' +
          '<div class="summary-box">' +
          '<strong>Protocolo:</strong> ' + r.protocolo + '<br>' +
          '<strong>Paciente:</strong> ' + r.paciente + ' (' + r.cpf + ')<br>' +
          '<strong>Medico:</strong> ' + r.medico + ' (CRM: ' + r.crm + ')<br>' +
          '<strong>Data:</strong> ' + r.data + ' - ' + r.hora + '<br>' +
          '<strong>Status:</strong> <span class="badge badge-danger">Cancelado</span>' +
          '</div>';
        return;
      }
      el.innerHTML =
        '<div class="summary-box">' +
        '<strong>Protocolo:</strong> ' + r.protocolo + '<br>' +
        '<strong>Paciente:</strong> ' + r.paciente + ' (' + r.cpf + ')<br>' +
        '<strong>Medico:</strong> ' + r.medico + ' (CRM: ' + r.crm + ')<br>' +
        '<strong>Especialidade:</strong> ' + r.especialidade + '<br>' +
        '<strong>Data:</strong> ' + r.data + ' - ' + r.hora + '<br>' +
        '<strong>Queixa:</strong> ' + (r.queixa || '-') + '<br>' +
        '<strong>Status:</strong> <span class="badge badge-success">Ativo</span>' +
        '</div>' +
        '<div class="btn-row"><button class="btn btn-danger" id="cancel-confirm-btn">Confirmar Cancelamento</button></div>';

      document.getElementById('cancel-confirm-btn').addEventListener('click', function () {
        var cr = call('bridge_agendamento_cancelar', [protoNum], ['number']);
        if (cr.ok) {
          toast('Agendamento cancelado com sucesso!');
          el.innerHTML = '<div class="alert alert-success">Agendamento protocolo ' + protoNum + ' cancelado.</div>';
          document.getElementById('cancel-proto').value = '';
        } else {
          toast(cr.erro || 'Erro ao cancelar.', 'error');
        }
      });
    });
  }

  // ── Grade Medico ─────────────────────────────────────────────────────

  function setupGradeMedico() {
    document.getElementById('grade-buscar-btn').addEventListener('click', function () {
      clearAllFieldErrors();
      var crm = document.getElementById('grade-crm').value.trim();
      if (!crm) { showFieldError('grade-crm', 'Informe o CRM'); return; }

      var med = call('bridge_medico_buscar_crm', [crm], ['string']);
      if (!med.encontrado) { toast('Medico nao encontrado.', 'error'); return; }

      var dispR = call('bridge_medico_disponibilidade_obter', [crm], ['string']);
      var disp = dispR.ok ? dispR.disp : null;

      var el = document.getElementById('grade-resultado');
      var html = '<div class="doctor-section"><div class="doctor-header">' +
        '<div class="doctor-avatar">' + med.nome.charAt(0).toUpperCase() + '</div>' +
        '<div><div class="doctor-name">' + med.nome + '</div><div class="doctor-spec">' + med.especialidade + ' - CRM: ' + med.crm + '</div></div>' +
        '</div></div>';

      var hoje = new Date();
      var diasNome = ['Dom', 'Seg', 'Ter', 'Qua', 'Qui', 'Sex', 'Sab'];
      var temDia = false;

      for (var gi = 0; gi < 14; gi++) {
        var gd = new Date(hoje); gd.setDate(hoje.getDate() + gi);
        var dow = gd.getDay();
        if (disp && !disp[dow][0] && !disp[dow][1]) continue;

        var gdStr = String(gd.getDate()).padStart(2, '0') + '/' + String(gd.getMonth() + 1).padStart(2, '0') + '/' + gd.getFullYear();
        var gdShort = String(gd.getDate()).padStart(2, '0') + '/' + String(gd.getMonth() + 1).padStart(2, '0');

        var gr = call('bridge_grade_obter_ou_criar', [crm, gdStr], ['string', 'string']);
        if (!gr.ok || !gr.slots || gr.slots.length === 0) continue;

        temDia = true;
        html += '<div class="date-card"><div class="date-card-header">' + gdShort + ' <span>' + diasNome[dow] + '</span></div><div style="padding:var(--space-sm) var(--space-md) var(--space-md);">';

        var turnoAtual = -1;
        for (var si = 0; si < gr.slots.length; si++) {
          var sl = gr.slots[si];
          var hVal = parseInt(sl.hora.substring(0, 2), 10);
          var turnoSl = hVal < 12 ? 0 : 1;
          if (turnoSl !== turnoAtual) {
            html += '<div class="turno-label">' + (turnoSl === 0 ? 'Manha' : 'Tarde') + '</div>';
            turnoAtual = turnoSl;
          }
          html += gradeLineHtml(sl);
        }
        html += '</div></div>';
      }

      if (!temDia) {
        html += '<div class="alert alert-info">Nenhum horario disponivel nos proximos 14 dias.</div>';
      }
      el.innerHTML = html;
    });
  }

  function gradeLineHtml(s) {
    var cls = s.ocupado ? '' : ' livre';
    var info = s.ocupado ? (s.paciente || '') + (s.queixa ? ' - ' + s.queixa : '') : 'Livre';
    return '<div class="occ-line"><span class="hora">' + s.hora + '</span><span class="' + cls + '">' + info + '</span></div>';
  }

  // ── Agenda do Dia ────────────────────────────────────────────────────

  function setupAgendaDia() {
    document.getElementById('agdia-buscar-btn').addEventListener('click', function () {
      clearAllFieldErrors();
      var data = normalizeDate(document.getElementById('agdia-data').value.trim());
      if (!data) { showFieldError('agdia-data', 'Informe a data'); return; }

      var agenda = call('bridge_agenda_dia', [data], ['string']);
      var el = document.getElementById('agdia-resultado');

      if (!agenda || agenda.length === 0) {
        el.innerHTML = emptyStateHtml('calendar', 'Nenhum medico com grade para esta data.');
        return;
      }

      var html = '';
      for (var i = 0; i < agenda.length; i++) {
        var med = agenda[i];
        html += '<div class="doctor-section">';
        html += '<div class="doctor-header">';
        html += '<div class="doctor-avatar">' + med.nome.charAt(0).toUpperCase() + '</div>';
        html += '<div><div class="doctor-name">' + med.nome + '</div><div class="doctor-spec">' + (med.especialidade || '') + ' - CRM: ' + (med.crm || '') + '</div></div>';
        html += '</div>';

        if (!med.slots || med.slots.length === 0) {
          html += '<p style="font-size:13px;color:var(--color-text-secondary);">Sem horarios.</p>';
        } else {
          var manha = [];
          var tarde = [];
          for (var s = 0; s < med.slots.length; s++) {
            var hora = parseInt(med.slots[s].hora.substring(0, 2), 10);
            if (hora < 12) manha.push(med.slots[s]);
            else tarde.push(med.slots[s]);
          }
          if (manha.length > 0) {
            html += '<div class="turno-label">Manha</div>';
            for (var m2 = 0; m2 < manha.length; m2++) html += gradeLineHtml(manha[m2]);
          }
          if (tarde.length > 0) {
            html += '<div class="turno-label">Tarde</div>';
            for (var t2 = 0; t2 < tarde.length; t2++) html += gradeLineHtml(tarde[t2]);
          }
        }
        html += '</div>';
      }
      el.innerHTML = html;
    });
  }

  // ── Demo Data ────────────────────────────────────────────────────────

  function populateDemoData() {
    var i, d;

    var medicos = [
      ['CG001', 'Dr. Ricardo Alves Mendonca', 0],
      ['CG002', 'Dra. Fernanda Costa Ribeiro', 0],
      ['PD001', 'Dr. Paulo Henrique Mendes', 1],
      ['PD002', 'Dra. Camila Torres Vieira', 1],
      ['GN001', 'Dra. Ana Lucia Rodrigues', 2],
      ['GN002', 'Dra. Patricia Sousa Lima', 2],
      ['OR001', 'Dr. Marcos Eduardo Oliveira', 3],
      ['OR002', 'Dr. Thiago Barros Santos', 3],
      ['GT001', 'Dr. Roberto Nascimento Filho', 4],
      ['GT002', 'Dra. Luciana Pires Monteiro', 4]
    ];
    for (i = 0; i < medicos.length; i++) {
      call('bridge_medico_cadastrar', medicos[i], ['string', 'string', 'number']);
    }

    for (i = 0; i < medicos.length; i++) {
      var crmDisp = medicos[i][0];
      for (d = 0; d < 7; d++) {
        call('bridge_medico_disponibilidade_alternar', [crmDisp, d, 0], ['string', 'number', 'number']);
        call('bridge_medico_disponibilidade_alternar', [crmDisp, d, 1], ['string', 'number', 'number']);
      }
    }

    var pacientes = [
      ['12345678901', 'Maria da Silva Santos', '15/03/1985', '(61)99801-2345'],
      ['23456789012', 'Joao Carlos Pereira', '22/07/1972', '(61)99802-3456'],
      ['34567890123', 'Ana Paula Ferreira', '08/11/1990', '(61)99803-4567'],
      ['45678901234', 'Carlos Eduardo Lima', '30/01/1965', '(61)99804-5678'],
      ['56789012345', 'Beatriz Almeida Souza', '14/06/2018', '(61)99805-6789'],
      ['67890123456', 'Roberto Vasconcelos', '05/09/1958', '(61)99806-7890'],
      ['78901234567', 'Luiz Henrique Barbosa', '12/04/1995', '(61)99807-8901'],
      ['89012345678', 'Camila Regina Torres', '20/08/2001', '(61)99808-9012'],
      ['90123456789', 'Jose Augusto Cardoso', '03/12/1948', '(61)99809-0123'],
      ['01234567890', 'Sandra Melo Carvalho', '27/05/1978', '(61)99800-1234']
    ];
    for (i = 0; i < pacientes.length; i++) {
      call('bridge_paciente_cadastrar', pacientes[i], ['string', 'string', 'string', 'string']);
    }

    var agendamentos = [
      ['12345678901', 'CG001', '19/06/2026', 2, 'dor de cabeca persistente ha uma semana'],
      ['23456789012', 'CG001', '19/06/2026', 4, 'pressao alta, tonturas pela manha'],
      ['34567890123', 'GN001', '19/06/2026', 1, 'dor pelvica ha duas semanas'],
      ['45678901234', 'OR001', '18/06/2026', 3, 'dor no joelho direito ao subir escadas'],
      ['56789012345', 'PD001', '18/06/2026', 0, 'febre e tosse ha tres dias, crianca de 8 anos'],
      ['67890123456', 'CG002', '22/06/2026', 5, 'consulta de rotina, hipertenso cronico'],
      ['78901234567', 'OR001', '22/06/2026', 2, 'dor no ombro esquerdo apos queda'],
      ['89012345678', 'CG001', '23/06/2026', 1, 'ansiedade e insonia ha tres semanas'],
      ['90123456789', 'GT001', '23/06/2026', 3, 'dor abdominal recorrente apos refeicoes'],
      ['01234567890', 'GN001', '24/06/2026', 0, 'retorno pos-consulta de novembro'],
      ['12345678901', 'CG002', '24/06/2026', 6, 'retorno - cefaleia controlada, renovar receita'],
      ['23456789012', 'CG001', '26/06/2026', 1, 'hipertensao, verificar medicacao'],
      ['45678901234', 'OR001', '26/06/2026', 3, 'dor no joelho, retorno apos raio-x'],
      ['56789012345', 'PD001', '26/06/2026', 0, 'tosse persistente, crianca melhorou pouco'],
      ['67890123456', 'GT001', '26/06/2026', 2, 'dor abdominal, resultado de exame'],
      ['89012345678', 'GN001', '26/06/2026', 1, 'preventivo anual'],
      ['78901234567', 'OR001', '29/06/2026', 4, 'dor no ombro, retorno apos fisioterapia'],
      ['90123456789', 'CG002', '29/06/2026', 2, 'diabetes tipo 2, controle trimestral'],
      ['34567890123', 'GN001', '30/06/2026', 0, 'resultado de ultrassom pelvico'],
      ['01234567890', 'CG001', '30/06/2026', 5, 'dor nas costas ha uma semana']
    ];
    for (i = 0; i < agendamentos.length; i++) {
      var ag = agendamentos[i];
      call('bridge_agendamento_criar', [ag[0], ag[1], ag[2], ag[3], ag[4]], ['string', 'string', 'string', 'number', 'string']);
    }

    call('bridge_agendamento_cancelar', [3], ['number']);
    call('bridge_agendamento_cancelar', [8], ['number']);
  }

  // ── Init ─────────────────────────────────────────────────────────────

  function init() {
    SusAgendaModule().then(function (mod) {
      Module = mod;
      document.getElementById('loading-screen').style.display = 'none';
      document.getElementById('app').style.display = 'flex';

      loadEspecialidades();
      populateDemoData();
      loadDashboard();

      // Navigation
      var navBtns = document.querySelectorAll('.nav-btn');
      for (var i = 0; i < navBtns.length; i++) {
        navBtns[i].addEventListener('click', function () {
          navigate(this.getAttribute('data-screen'));
        });
      }

      // Mobile sidebar toggle
      var toggleBtn = document.getElementById('sidebar-toggle');
      var sidebarEl = document.getElementById('sidebar');
      var overlayEl = document.getElementById('sidebar-overlay');
      if (toggleBtn && sidebarEl) {
        toggleBtn.addEventListener('click', function () {
          sidebarEl.classList.toggle('open');
          if (overlayEl) overlayEl.classList.toggle('open');
        });
      }
      if (overlayEl) {
        overlayEl.addEventListener('click', function () {
          if (sidebarEl) sidebarEl.classList.remove('open');
          overlayEl.classList.remove('open');
        });
      }

      // Setup all screen handlers
      setupPacCadastrar();
      setupMedCadastrar();
      setupDisp();
      setupCancelar();
      setupGradeMedico();
      setupAgendaDia();
      setupHistorico();
    });
  }

  init();
})();
