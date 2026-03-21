// В MainWindow.cpp

void MainWindow::onShowREPL() {
    consoleWidget->showREPLTab();
    consoleWidget->setREPLEnabled(true);
}

void MainWindow::onExecuteREPLCommand(const QString& command) {
    consoleWidget->executeREPLCommand(command);
}

// В ConsoleWidget.cpp

void ConsoleWidget::showREPLTab() {
    if (!replTab) {
        replTab = new ConsoleTab(ConsoleTabType::REPL, this);
        replTab->setPlaceholderText("Proxima REPL - Type 'help' for commands...");
        tabs[ConsoleTabType::REPL] = replTab;
        tabWidget->addTab(replTab, QIcon(":/icons/repl.svg"), "REPL");
        
        // Инициализация REPL
        repl = new REPL(this);
        repl->initialize();
        
        // Подключение сигналов
        connect(repl, &REPL::outputReceived, this, [this](const QString& output) {
            if (replTab) {
                replTab->appendText(output);
            }
        });
        
        connect(repl, &REPL::errorReceived, this, [this](const QString& error) {
            if (replTab) {
                replTab->appendText(error, MessageType::Error);
            }
        });
        
        connect(repl, &REPL::promptDisplayed, this, [this]() {
            if (replTab) {
                replTab->appendText(repl->getPrompt());
            }
        });
    }
    
    int index = tabWidget->indexOf(replTab);
    if (index >= 0) {
        tabWidget->setCurrentIndex(index);
    }
}

void ConsoleWidget::executeREPLCommand(const QString& command) {
    if (repl && replEnabled) {
        QString result = repl->execute(command);
        if (!result.isEmpty() && replTab) {
            replTab->appendText(result);
        }
    }
}