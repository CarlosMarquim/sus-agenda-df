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

  // ── Toast ────────────────────────────────────────────────────────────

  function toast(msg, type) {
    var el = document.createElement('div');
    el.className = 'toast ' + (type === 'error' ? 'toast-error' : 'toast-success');
    el.textContent = msg;
    document.body.appendChild(el);
    setTimeout(function () { el.remove(); }, 3000);
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
      hojeEl.innerHTML = '<p style="color:var(--color-text-secondary);font-size:14px;">Nenhuma consulta agendada para hoje.</p>';
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
      hojeEl.innerHTML = '<p style="color:var(--color-text-secondary);font-size:14px;">Nenhuma consulta agendada para hoje.</p>';
      return;
    }
    hojeEl.innerHTML =
      '<table><thead><tr><th>Hora</th><th>Medico</th><th>Especialidade</th><th>Paciente</th><th>Queixa</th></tr></thead><tbody>' +
      rows + '</tbody></table>';
  }

  // ── Pacientes: Cadastrar ─────────────────────────────────────────────

  function setupPacCadastrar() {
    document.getElementById('pac-cadastrar-btn').addEventListener('click', function () {
      var cpf = document.getElementById('pac-cpf').value.trim();
      var nome = document.getElementById('pac-nome').value.trim();
      var nasc = document.getElementById('pac-nasc').value.trim();
      var tel = document.getElementById('pac-tel').value.trim();
      if (!cpf || !nome || !nasc || !tel) {
        toast('Preencha todos os campos.', 'error');
        return;
      }
      var r = call('bridge_paciente_cadastrar', [cpf, nome, nasc, tel], ['string', 'string', 'string', 'string']);
      if (r.ok) {
        toast('Paciente cadastrado com sucesso!');
        document.getElementById('pac-cpf').value = '';
        document.getElementById('pac-nome').value = '';
        document.getElementById('pac-nasc').value = '';
        document.getElementById('pac-tel').value = '';
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
      el.innerHTML = '<p style="color:var(--color-text-secondary);font-size:14px;">Nenhum paciente cadastrado.</p>';
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
      var cpf = document.getElementById('hist-cpf').value.trim();
      if (!cpf) { toast('Informe o CPF.', 'error'); return; }
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
        html += '<p style="color:var(--color-text-secondary);font-size:14px;">Nenhum agendamento encontrado.</p>';
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
      var crm = document.getElementById('med-crm').value.trim();
      var nome = document.getElementById('med-nome').value.trim();
      var espIdx = parseInt(document.getElementById('med-esp').value, 10);
      if (!crm || !nome) {
        toast('Preencha CRM e Nome.', 'error');
        return;
      }
      var r = call('bridge_medico_cadastrar', [crm, nome, espIdx], ['string', 'string', 'number']);
      if (r.ok) {
        toast('Medico cadastrado com sucesso!');
        document.getElementById('med-crm').value = '';
        document.getElementById('med-nome').value = '';
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
      el.innerHTML = '<p style="color:var(--color-text-secondary);font-size:14px;">Nenhum medico cadastrado.</p>';
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
      var crm = document.getElementById('disp-crm').value.trim();
      if (!crm) { toast('Informe o CRM.', 'error'); return; }
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

  var AG_STEPS = 6; // 0=CPF, 1=especialidade, 2=medico, 3=data+grade+slot, 4=queixa, 5=confirmar

  function renderSteps() {
    var el = document.getElementById('ag-steps');
    var html = '';
    for (var i = 0; i < AG_STEPS; i++) {
      var cls = 'step-dot';
      if (i < agState.step) cls += ' done';
      else if (i === agState.step) cls += ' active';
      html += '<div class="' + cls + '"></div>';
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
      // Step 0: CPF
      el.innerHTML =
        '<h3>Identificar Paciente</h3>' +
        '<div style="max-width:400px;">' +
        '<div class="form-group"><label>CPF</label><input type="text" id="ag-cpf" placeholder="00000000000" maxlength="11"></div>' +
        '<div class="btn-row"><button class="btn btn-primary" id="ag-cpf-btn">Buscar</button></div>' +
        '</div>';
      document.getElementById('ag-cpf-btn').addEventListener('click', function () {
        var cpf = document.getElementById('ag-cpf').value.trim();
        if (!cpf) { toast('Informe o CPF.', 'error'); return; }
        var r = call('bridge_paciente_buscar_cpf', [cpf], ['string']);
        if (!r.encontrado) { toast('Paciente nao encontrado.', 'error'); return; }
        agState.cpf = r.cpf;
        agState.pacNome = r.nome;
        agState.step = 1;
        renderAgStep();
      });

    } else if (agState.step === 1) {
      // Step 1: Especialidade
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
      // Step 2: Medico
      var medicos = call('bridge_medico_buscar_especialidade', [agState.espIdx], ['number']);
      var html2 = '<h3>Selecionar Medico</h3>' +
        '<div class="summary-box"><strong>Paciente:</strong> ' + agState.pacNome + '<br><strong>Especialidade:</strong> ' + agState.espNome + '</div>';
      if (!medicos || medicos.length === 0) {
        html2 += '<div class="alert alert-danger">Nenhum medico encontrado para esta especialidade.</div>';
        html2 += '<div class="btn-row"><button class="btn btn-secondary" id="ag-back-2">Voltar</button></div>';
        el.innerHTML = html2;
        document.getElementById('ag-back-2').addEventListener('click', function () {
          agState.step = 1;
          renderAgStep();
        });
        return;
      }
      html2 += '<div style="margin-top:16px;">';
      for (var k = 0; k < medicos.length; k++) {
        var m = medicos[k];
        html2 += '<div class="doctor-section" style="cursor:pointer;" data-crm="' + m.crm + '" data-nome="' + m.nome + '">' +
          '<div class="doctor-header">' +
          '<div class="doctor-avatar">' + m.nome.charAt(0).toUpperCase() + '</div>' +
          '<div><div class="doctor-name">' + m.nome + '</div><div class="doctor-spec">CRM: ' + m.crm + '</div></div>' +
          '</div></div>';
      }
      html2 += '</div>';
      html2 += '<div class="btn-row"><button class="btn btn-secondary" id="ag-back-2">Voltar</button></div>';
      el.innerHTML = html2;

      var docs = document.querySelectorAll('#ag-content .doctor-section');
      for (var l = 0; l < docs.length; l++) {
        docs[l].addEventListener('click', function () {
          agState.crm = this.getAttribute('data-crm');
          agState.medNome = this.getAttribute('data-nome');
          agState.step = 3;
          renderAgStep();
        });
      }
      document.getElementById('ag-back-2').addEventListener('click', function () {
        agState.step = 1;
        renderAgStep();
      });

    } else if (agState.step === 3) {
      // Step 3: Data + Grade + Slot
      el.innerHTML =
        '<h3>Selecionar Data e Horario</h3>' +
        '<div class="summary-box"><strong>Paciente:</strong> ' + agState.pacNome + '<br><strong>Medico:</strong> ' + agState.medNome + ' (' + agState.espNome + ')</div>' +
        '<div style="max-width:400px;margin-top:16px;">' +
        '<div class="form-group"><label>Data da Consulta</label><input type="text" id="ag-data" placeholder="DD/MM/AAAA"></div>' +
        '<div class="btn-row"><button class="btn btn-primary" id="ag-grade-btn">Ver Horarios</button></div>' +
        '</div>' +
        '<div id="ag-slots" style="margin-top:16px;"></div>' +
        '<div class="btn-row"><button class="btn btn-secondary" id="ag-back-3">Voltar</button></div>';

      document.getElementById('ag-grade-btn').addEventListener('click', function () {
        var data = document.getElementById('ag-data').value.trim();
        if (!data) { toast('Informe a data.', 'error'); return; }
        agState.data = data;
        var r = call('bridge_grade_obter_ou_criar', [agState.crm, data], ['string', 'string']);
        if (!r.ok) {
          toast('Erro ao obter grade.', 'error');
          return;
        }
        renderSlots(r.slots);
      });

      document.getElementById('ag-back-3').addEventListener('click', function () {
        agState.step = 2;
        renderAgStep();
      });

    } else if (agState.step === 4) {
      // Step 4: Queixa
      el.innerHTML =
        '<h3>Informar Queixa</h3>' +
        '<div class="summary-box"><strong>Paciente:</strong> ' + agState.pacNome + '<br><strong>Medico:</strong> ' + agState.medNome + '<br><strong>Data:</strong> ' + agState.data + '<br><strong>Horario:</strong> ' + agState.slotHora + '</div>' +
        '<div style="max-width:480px;margin-top:16px;">' +
        '<div class="form-group"><label>Queixa / Motivo da Consulta</label><textarea id="ag-queixa" placeholder="Descreva a queixa do paciente"></textarea></div>' +
        '<div class="btn-row"><button class="btn btn-secondary" id="ag-back-4">Voltar</button><button class="btn btn-primary" id="ag-queixa-btn">Continuar</button></div>' +
        '</div>';

      document.getElementById('ag-queixa-btn').addEventListener('click', function () {
        agState.queixa = document.getElementById('ag-queixa').value.trim();
        if (!agState.queixa) { toast('Informe a queixa.', 'error'); return; }
        agState.step = 5;
        renderAgStep();
      });
      document.getElementById('ag-back-4').addEventListener('click', function () {
        agState.step = 3;
        renderAgStep();
      });

    } else if (agState.step === 5) {
      // Step 5: Confirmar
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
        '<div class="btn-row"><button class="btn btn-secondary" id="ag-back-5">Voltar</button><button class="btn btn-primary" id="ag-confirm-btn">Confirmar</button></div>';

      document.getElementById('ag-confirm-btn').addEventListener('click', function () {
        var r = call('bridge_agendamento_criar', [agState.cpf, agState.crm, agState.data, agState.slotIdx, agState.queixa], ['string', 'string', 'string', 'number', 'string']);
        if (r.ok) {
          toast('Agendamento criado! Protocolo: ' + r.protocolo);
          el.innerHTML =
            '<div class="alert alert-success">Agendamento confirmado com sucesso!<br>Protocolo: <strong>' + r.protocolo + '</strong></div>' +
            '<div class="btn-row"><button class="btn btn-primary" id="ag-novo-btn">Novo Agendamento</button></div>';
          document.getElementById('ag-novo-btn').addEventListener('click', initAgendamento);
        } else {
          toast(r.erro || 'Erro ao agendar.', 'error');
        }
      });
      document.getElementById('ag-back-5').addEventListener('click', function () {
        agState.step = 4;
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

    // Separate morning and afternoon
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

    // Bind slot clicks
    var slotCards = el.querySelectorAll('.slot-card:not(.occupied)');
    for (var j = 0; j < slotCards.length; j++) {
      slotCards[j].addEventListener('click', function () {
        // Deselect all
        var all = el.querySelectorAll('.slot-card');
        for (var x = 0; x < all.length; x++) all[x].classList.remove('selected');
        this.classList.add('selected');
        agState.slotIdx = parseInt(this.getAttribute('data-idx'), 10);
        agState.slotHora = this.getAttribute('data-hora');

        // Add continue button if not already present
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
      var proto = document.getElementById('cancel-proto').value.trim();
      if (!proto) { toast('Informe o protocolo.', 'error'); return; }
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
      var crm = document.getElementById('grade-crm').value.trim();
      var data = document.getElementById('grade-data').value.trim();
      if (!crm || !data) { toast('Preencha CRM e data.', 'error'); return; }

      var med = call('bridge_medico_buscar_crm', [crm], ['string']);
      if (!med.encontrado) { toast('Medico nao encontrado.', 'error'); return; }

      var r = call('bridge_grade_obter_ou_criar', [crm, data], ['string', 'string']);
      var el = document.getElementById('grade-resultado');
      if (!r.ok) { toast('Erro ao obter grade.', 'error'); return; }

      var html = '<div class="doctor-section"><div class="doctor-header">' +
        '<div class="doctor-avatar">' + med.nome.charAt(0).toUpperCase() + '</div>' +
        '<div><div class="doctor-name">' + med.nome + '</div><div class="doctor-spec">' + med.especialidade + ' - CRM: ' + med.crm + '</div></div>' +
        '</div></div>';

      if (!r.slots || r.slots.length === 0) {
        html += '<div class="alert alert-info">Nenhum horario disponivel nesta data.</div>';
        el.innerHTML = html;
        return;
      }

      // Separate morning/afternoon
      var manha = [];
      var tarde = [];
      for (var i = 0; i < r.slots.length; i++) {
        var hora = parseInt(r.slots[i].hora.substring(0, 2), 10);
        if (hora < 12) manha.push(r.slots[i]);
        else tarde.push(r.slots[i]);
      }

      if (manha.length > 0) {
        html += '<div class="turno-label">Manha</div>';
        for (var m = 0; m < manha.length; m++) {
          html += gradeLineHtml(manha[m]);
        }
      }
      if (tarde.length > 0) {
        html += '<div class="turno-label">Tarde</div>';
        for (var t = 0; t < tarde.length; t++) {
          html += gradeLineHtml(tarde[t]);
        }
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
      var data = document.getElementById('agdia-data').value.trim();
      if (!data) { toast('Informe a data.', 'error'); return; }

      var agenda = call('bridge_agenda_dia', [data], ['string']);
      var el = document.getElementById('agdia-resultado');

      if (!agenda || agenda.length === 0) {
        el.innerHTML = '<div class="alert alert-info">Nenhum medico com grade para esta data.</div>';
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
            for (var m = 0; m < manha.length; m++) html += gradeLineHtml(manha[m]);
          }
          if (tarde.length > 0) {
            html += '<div class="turno-label">Tarde</div>';
            for (var t = 0; t < tarde.length; t++) html += gradeLineHtml(tarde[t]);
          }
        }
        html += '</div>';
      }
      el.innerHTML = html;
    });
  }

  // ── Init ─────────────────────────────────────────────────────────────

  function init() {
    SusAgendaModule().then(function (mod) {
      Module = mod;
      document.getElementById('loading-screen').style.display = 'none';
      document.getElementById('app').style.display = 'flex';

      loadEspecialidades();
      loadDashboard();

      // Navigation
      var navBtns = document.querySelectorAll('.nav-btn');
      for (var i = 0; i < navBtns.length; i++) {
        navBtns[i].addEventListener('click', function () {
          navigate(this.getAttribute('data-screen'));
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
