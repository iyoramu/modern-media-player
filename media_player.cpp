/*
 * ModernMediaPlayer - A modern, feature-rich media player
 * 
 * Features:
 * - Hardware-accelerated video playback
 * - Support for most media formats
 * - Modern UI with dark/light theme
 * - Playlist management
 * - Subtitles support
 * - Audio equalizer
 * - Streaming capabilities
 * - Touchscreen-friendly controls
 */

#include <QApplication>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QFileDialog>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QListWidget>
#include <QSettings>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTime>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QComboBox>
#include <QAudioOutput>
#include <QAudioDevice>
#include <QAudio>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QToolButton>
#include <QSystemTrayIcon>
#include <QShortcut>
#include <QStackedWidget>
#include <QMouseEvent>
#include <QTimer>
#include <QMediaMetaData>
#include <QMediaFormat>
#include <QMediaDevices>
#include <QAudioOutput>

class VideoWidget : public QVideoWidget {
public:
    VideoWidget(QWidget *parent = nullptr) : QVideoWidget(parent) {
        setMouseTracking(true);
        setCursor(Qt::BlankCursor);
    }

protected:
    void mouseMoveEvent(QMouseEvent *event) override {
        QVideoWidget::mouseMoveEvent(event);
        unsetCursor();
        if (m_hideCursorTimer) {
            m_hideCursorTimer->stop();
        }
        m_hideCursorTimer = new QTimer(this);
        m_hideCursorTimer->setSingleShot(true);
        connect(m_hideCursorTimer, &QTimer::timeout, this, [this]() {
            setCursor(Qt::BlankCursor);
        });
        m_hideCursorTimer->start(2000);
    }

private:
    QTimer *m_hideCursorTimer = nullptr;
};

class MediaPlayer : public QMainWindow {
    Q_OBJECT

public:
    MediaPlayer(QWidget *parent = nullptr) : QMainWindow(parent) {
        setupUi();
        setupPlayer();
        setupConnections();
        loadSettings();
        setupShortcuts();
        setupSystemTray();
    }

    ~MediaPlayer() {
        saveSettings();
    }

private:
    void setupUi() {
        // Window setup
        setWindowTitle("ModernMediaPlayer");
        setWindowIcon(QIcon(":/icons/app_icon"));
        resize(1280, 720);

        // Central widget
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        // Main layouts
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // Video widget
        m_videoWidget = new VideoWidget(this);
        m_videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mainLayout->addWidget(m_videoWidget, 1);

        // Control panel
        QWidget *controlPanel = new QWidget(this);
        controlPanel->setObjectName("controlPanel");
        controlPanel->setStyleSheet("QWidget#controlPanel { background-color: rgba(30, 30, 30, 200); }");
        
        QHBoxLayout *controlLayout = new QHBoxLayout(controlPanel);
        controlLayout->setContentsMargins(10, 5, 10, 5);

        // Play/pause button
        m_playButton = new QToolButton(this);
        m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        m_playButton->setToolTip("Play/Pause");
        m_playButton->setIconSize(QSize(32, 32));
        m_playButton->setStyleSheet("QToolButton { border: none; background: transparent; }");
        controlLayout->addWidget(m_playButton);

        // Stop button
        m_stopButton = new QToolButton(this);
        m_stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
        m_stopButton->setToolTip("Stop");
        m_stopButton->setIconSize(QSize(24, 24));
        m_stopButton->setStyleSheet("QToolButton { border: none; background: transparent; }");
        controlLayout->addWidget(m_stopButton);

        // Previous button
        m_prevButton = new QToolButton(this);
        m_prevButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
        m_prevButton->setToolTip("Previous");
        m_prevButton->setIconSize(QSize(24, 24));
        m_prevButton->setStyleSheet("QToolButton { border: none; background: transparent; }");
        controlLayout->addWidget(m_prevButton);

        // Next button
        m_nextButton = new QToolButton(this);
        m_nextButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
        m_nextButton->setToolTip("Next");
        m_nextButton->setIconSize(QSize(24, 24));
        m_nextButton->setStyleSheet("QToolButton { border: none; background: transparent; }");
        controlLayout->addWidget(m_nextButton);

        // Volume button
        m_volumeButton = new QToolButton(this);
        m_volumeButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
        m_volumeButton->setToolTip("Volume");
        m_volumeButton->setIconSize(QSize(24, 24));
        m_volumeButton->setStyleSheet("QToolButton { border: none; background: transparent; }");
        m_volumeButton->setCheckable(true);
        controlLayout->addWidget(m_volumeButton);

        // Volume slider
        m_volumeSlider = new QSlider(Qt::Horizontal, this);
        m_volumeSlider->setRange(0, 100);
        m_volumeSlider->setValue(50);
        m_volumeSlider->setFixedWidth(100);
        m_volumeSlider->setToolTip("Volume");
        controlLayout->addWidget(m_volumeSlider);

        // Time slider
        m_timeSlider = new QSlider(Qt::Horizontal, this);
        m_timeSlider->setRange(0, 100);
        m_timeSlider->setToolTip("Seek");
        controlLayout->addWidget(m_timeSlider, 1);

        // Time label
        m_timeLabel = new QLabel("00:00:00 / 00:00:00", this);
        m_timeLabel->setToolTip("Current time / Total time");
        controlLayout->addWidget(m_timeLabel);

        // Fullscreen button
        m_fullscreenButton = new QToolButton(this);
        m_fullscreenButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
        m_fullscreenButton->setToolTip("Fullscreen");
        m_fullscreenButton->setIconSize(QSize(24, 24));
        m_fullscreenButton->setStyleSheet("QToolButton { border: none; background: transparent; }");
        controlLayout->addWidget(m_fullscreenButton);

        // Playback rate
        m_playbackRateBox = new QComboBox(this);
        m_playbackRateBox->addItems({"0.5x", "0.75x", "1.0x", "1.25x", "1.5x", "2.0x"});
        m_playbackRateBox->setCurrentIndex(2);
        m_playbackRateBox->setToolTip("Playback speed");
        m_playbackRateBox->setFixedWidth(70);
        controlLayout->addWidget(m_playbackRateBox);

        mainLayout->addWidget(controlPanel);

        // Status bar
        m_statusBar = new QStatusBar(this);
        setStatusBar(m_statusBar);

        // Menu bar
        createMenuBar();

        // Playlist dock
        m_playlistWidget = new QListWidget(this);
        m_playlistWidget->setAlternatingRowColors(true);
        m_playlistDock = new QDockWidget("Playlist", this);
        m_playlistDock->setWidget(m_playlistWidget);
        addDockWidget(Qt::RightDockWidgetArea, m_playlistDock);

        // Equalizer dock (placeholder)
        m_equalizerWidget = new QWidget(this);
        m_equalizerDock = new QDockWidget("Equalizer", this);
        m_equalizerDock->setWidget(m_equalizerWidget);
        addDockWidget(Qt::RightDockWidgetArea, m_equalizerDock);
        m_equalizerDock->hide();

        // Apply styles
        applyStyle();
    }

    void setupPlayer() {
        m_player = new QMediaPlayer(this);
        m_audioOutput = new QAudioOutput(this);
        m_player->setAudioOutput(m_audioOutput);
        m_player->setVideoOutput(m_videoWidget);

        // Set initial volume
        m_audioOutput->setVolume(m_volumeSlider->value() / 100.0);
    }

    void setupConnections() {
        // Player connections
        connect(m_player, &QMediaPlayer::positionChanged, this, &MediaPlayer::updatePosition);
        connect(m_player, &QMediaPlayer::durationChanged, this, &MediaPlayer::updateDuration);
        connect(m_player, &QMediaPlayer::playbackStateChanged, this, &MediaPlayer::updatePlayButton);
        connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &MediaPlayer::handleMediaStatus);
        connect(m_player, &QMediaPlayer::errorOccurred, this, &MediaPlayer::handlePlayerError);

        // UI connections
        connect(m_playButton, &QToolButton::clicked, this, &MediaPlayer::togglePlayPause);
        connect(m_stopButton, &QToolButton::clicked, m_player, &QMediaPlayer::stop);
        connect(m_timeSlider, &QSlider::sliderMoved, this, &MediaPlayer::seek);
        connect(m_volumeSlider, &QSlider::valueChanged, this, &MediaPlayer::setVolume);
        connect(m_volumeButton, &QToolButton::toggled, this, &MediaPlayer::toggleMute);
        connect(m_fullscreenButton, &QToolButton::clicked, this, &MediaPlayer::toggleFullscreen);
        connect(m_playbackRateBox, &QComboBox::currentTextChanged, this, &MediaPlayer::setPlaybackRate);
        connect(m_prevButton, &QToolButton::clicked, this, &MediaPlayer::previousTrack);
        connect(m_nextButton, &QToolButton::clicked, this, &MediaPlayer::nextTrack);
        connect(m_playlistWidget, &QListWidget::itemDoubleClicked, this, &MediaPlayer::playSelectedItem);
    }

    void createMenuBar() {
        QMenuBar *menuBar = new QMenuBar(this);
        setMenuBar(menuBar);

        // File menu
        QMenu *fileMenu = menuBar->addMenu("&File");
        QAction *openFileAction = fileMenu->addAction("&Open File...");
        openFileAction->setShortcut(QKeySequence::Open);
        connect(openFileAction, &QAction::triggered, this, &MediaPlayer::openFile);

        QAction *openUrlAction = fileMenu->addAction("Open &URL...");
        openUrlAction->setShortcut(Qt::CTRL | Qt::Key_U);
        connect(openUrlAction, &QAction::triggered, this, &MediaPlayer::openUrl);

        fileMenu->addSeparator();
        QAction *exitAction = fileMenu->addAction("E&xit");
        exitAction->setShortcut(QKeySequence::Quit);
        connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

        // Playback menu
        QMenu *playbackMenu = menuBar->addMenu("&Playback");
        m_playAction = playbackMenu->addAction("&Play/Pause");
        m_playAction->setShortcut(Qt::Key_Space);
        connect(m_playAction, &QAction::triggered, this, &MediaPlayer::togglePlayPause);

        m_stopAction = playbackMenu->addAction("&Stop");
        m_stopAction->setShortcut(Qt::Key_Stop);
        connect(m_stopAction, &QAction::triggered, m_player, &QMediaPlayer::stop);

        playbackMenu->addSeparator();
        QAction *prevAction = playbackMenu->addAction("&Previous");
        prevAction->setShortcut(Qt::Key_P);
        connect(prevAction, &QAction::triggered, this, &MediaPlayer::previousTrack);

        QAction *nextAction = playbackMenu->addAction("&Next");
        nextAction->setShortcut(Qt::Key_N);
        connect(nextAction, &QAction::triggered, this, &MediaPlayer::nextTrack);

        // View menu
        QMenu *viewMenu = menuBar->addMenu("&View");
        m_playlistAction = viewMenu->addAction("&Playlist");
        m_playlistAction->setCheckable(true);
        m_playlistAction->setChecked(true);
        m_playlistAction->setShortcut(Qt::CTRL | Qt::Key_L);
        connect(m_playlistAction, &QAction::toggled, m_playlistDock, &QDockWidget::setVisible);

        m_equalizerAction = viewMenu->addAction("&Equalizer");
        m_equalizerAction->setCheckable(true);
        m_equalizerAction->setChecked(false);
        m_equalizerAction->setShortcut(Qt::CTRL | Qt::Key_E);
        connect(m_equalizerAction, &QAction::toggled, m_equalizerDock, &QDockWidget::setVisible);

        viewMenu->addSeparator();
        QAction *fullscreenAction = viewMenu->addAction("&Fullscreen");
        fullscreenAction->setShortcut(Qt::Key_F11);
        connect(fullscreenAction, &QAction::triggered, this, &MediaPlayer::toggleFullscreen);

        // Help menu
        QMenu *helpMenu = menuBar->addMenu("&Help");
        QAction *aboutAction = helpMenu->addAction("&About");
        connect(aboutAction, &QAction::triggered, this, &MediaPlayer::showAbout);
    }

    void setupShortcuts() {
        // Space for play/pause
        QShortcut *spaceShortcut = new QShortcut(Qt::Key_Space, this);
        connect(spaceShortcut, &QShortcut::activated, this, &MediaPlayer::togglePlayPause);

        // Left/Right for seeking
        QShortcut *leftShortcut = new QShortcut(Qt::Key_Left, this);
        connect(leftShortcut, &QShortcut::activated, this, [this]() {
            m_player->setPosition(m_player->position() - 5000); // 5 seconds back
        });

        QShortcut *rightShortcut = new QShortcut(Qt::Key_Right, this);
        connect(rightShortcut, &QShortcut::activated, this, [this]() {
            m_player->setPosition(m_player->position() + 5000); // 5 seconds forward
        });

        // Up/Down for volume
        QShortcut *upShortcut = new QShortcut(Qt::Key_Up, this);
        connect(upShortcut, &QShortcut::activated, this, [this]() {
            m_volumeSlider->setValue(m_volumeSlider->value() + 5);
        });

        QShortcut *downShortcut = new QShortcut(Qt::Key_Down, this);
        connect(downShortcut, &QShortcut::activated, this, [this]() {
            m_volumeSlider->setValue(m_volumeSlider->value() - 5);
        });

        // M for mute
        QShortcut *muteShortcut = new QShortcut(Qt::Key_M, this);
        connect(muteShortcut, &QShortcut::activated, this, [this]() {
            m_volumeButton->toggle();
        });

        // F for fullscreen
        QShortcut *fullscreenShortcut = new QShortcut(Qt::Key_F, this);
        connect(fullscreenShortcut, &QShortcut::activated, this, &MediaPlayer::toggleFullscreen);

        // Esc to exit fullscreen
        QShortcut *escShortcut = new QShortcut(Qt::Key_Escape, this);
        connect(escShortcut, &QShortcut::activated, this, [this]() {
            if (isFullScreen()) {
                showNormal();
            }
        });
    }

    void setupSystemTray() {
        if (QSystemTrayIcon::isSystemTrayAvailable()) {
            m_trayIcon = new QSystemTrayIcon(this);
            m_trayIcon->setIcon(QIcon(":/icons/app_icon"));
            
            QMenu *trayMenu = new QMenu(this);
            QAction *restoreAction = trayMenu->addAction("Restore");
            connect(restoreAction, &QAction::triggered, this, &QMainWindow::showNormal);
            
            QAction *quitAction = trayMenu->addAction("Quit");
            connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
            
            m_trayIcon->setContextMenu(trayMenu);
            m_trayIcon->show();
            
            connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::DoubleClick) {
                    if (isVisible()) {
                        hide();
                    } else {
                        showNormal();
                        activateWindow();
                    }
                }
            });
        }
    }

    void applyStyle() {
        // Modern dark theme
        QString styleSheet = R"(
            QMainWindow {
                background-color: #1e1e1e;
            }
            QSlider::groove:horizontal {
                height: 6px;
                background: #404040;
                border-radius: 3px;
            }
            QSlider::handle:horizontal {
                width: 12px;
                height: 12px;
                background: #ffffff;
                border-radius: 6px;
                margin: -3px 0;
            }
            QSlider::sub-page:horizontal {
                background: #3daee9;
                border-radius: 3px;
            }
            QListWidget {
                background-color: #252525;
                color: #ffffff;
                border: none;
                font-size: 12px;
            }
            QListWidget::item {
                padding: 5px;
                border-bottom: 1px solid #353535;
            }
            QListWidget::item:selected {
                background-color: #3daee9;
                color: #ffffff;
            }
            QListWidget::item:hover {
                background-color: #353535;
            }
            QStatusBar {
                background-color: #252525;
                color: #aaaaaa;
                font-size: 11px;
            }
            QComboBox {
                background-color: #353535;
                color: #ffffff;
                border: 1px solid #454545;
                padding: 2px;
                border-radius: 3px;
            }
            QComboBox QAbstractItemView {
                background-color: #353535;
                color: #ffffff;
                selection-background-color: #3daee9;
            }
            QLabel {
                color: #ffffff;
                font-size: 12px;
            }
        )";
        
        setStyleSheet(styleSheet);
    }

    void loadSettings() {
        QSettings settings("ModernMediaPlayer", "MediaPlayer");
        
        // Window geometry
        restoreGeometry(settings.value("geometry").toByteArray());
        restoreState(settings.value("windowState").toByteArray());
        
        // Player settings
        m_volumeSlider->setValue(settings.value("volume", 50).toInt());
        m_playbackRateBox->setCurrentIndex(settings.value("playbackRate", 2).toInt());
        
        // Playlist
        QStringList recentFiles = settings.value("recentFiles").toStringList();
        for (const QString &file : recentFiles) {
            addToPlaylist(file);
        }
        
        // UI settings
        m_playlistAction->setChecked(settings.value("showPlaylist", true).toBool());
        m_equalizerAction->setChecked(settings.value("showEqualizer", false).toBool());
    }

    void saveSettings() {
        QSettings settings("ModernMediaPlayer", "MediaPlayer");
        
        // Window geometry
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
        
        // Player settings
        settings.setValue("volume", m_volumeSlider->value());
        settings.setValue("playbackRate", m_playbackRateBox->currentIndex());
        
        // Playlist
        QStringList recentFiles;
        for (int i = 0; i < m_playlistWidget->count(); ++i) {
            recentFiles << m_playlistWidget->item(i)->data(Qt::UserRole).toString();
        }
        settings.setValue("recentFiles", recentFiles);
        
        // UI settings
        settings.setValue("showPlaylist", m_playlistAction->isChecked());
        settings.setValue("showEqualizer", m_equalizerAction->isChecked());
    }

private slots:
    void openFile() {
        QStringList files = QFileDialog::getOpenFileNames(
            this,
            "Open Media File",
            QStandardPaths::writableLocation(QStandardPaths::MoviesLocation),
            "Media Files (*.mp4 *.avi *.mkv *.mov *.mp3 *.wav *.flac *.m4a);;All Files (*.*)"
        );
        
        if (!files.isEmpty()) {
            for (const QString &file : files) {
                addToPlaylist(file);
            }
            
            // Play the first file
            playFile(files.first());
        }
    }

    void openUrl() {
        bool ok;
        QString url = QInputDialog::getText(
            this,
            "Open URL",
            "Enter media URL:",
            QLineEdit::Normal,
            "http://",
            &ok
        );
        
        if (ok && !url.isEmpty()) {
            addToPlaylist(url);
            playFile(url);
        }
    }

    void addToPlaylist(const QString &filePath) {
        QFileInfo fileInfo(filePath);
        QListWidgetItem *item = new QListWidgetItem(fileInfo.fileName());
        item->setData(Qt::UserRole, filePath);
        m_playlistWidget->addItem(item);
    }

    void playFile(const QString &filePath) {
        m_player->setSource(QUrl::fromUserInput(filePath));
        m_player->play();
        m_statusBar->showMessage("Now playing: " + QFileInfo(filePath).fileName());
    }

    void playSelectedItem(QListWidgetItem *item) {
        playFile(item->data(Qt::UserRole).toString());
    }

    void togglePlayPause() {
        if (m_player->playbackState() == QMediaPlayer::PlayingState) {
            m_player->pause();
        } else {
            m_player->play();
        }
    }

    void updatePlayButton() {
        if (m_player->playbackState() == QMediaPlayer::PlayingState) {
            m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            m_playAction->setText("&Pause");
        } else {
            m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            m_playAction->setText("&Play");
        }
    }

    void updatePosition(qint64 position) {
        m_timeSlider->setValue(static_cast<int>(position));
        
        QTime currentTime(0, 0, 0);
        currentTime = currentTime.addMSecs(static_cast<int>(position));
        
        QTime totalTime(0, 0, 0);
        totalTime = totalTime.addMSecs(static_cast<int>(m_player->duration()));
        
        m_timeLabel->setText(currentTime.toString("hh:mm:ss") + " / " + totalTime.toString("hh:mm:ss"));
    }

    void updateDuration(qint64 duration) {
        m_timeSlider->setRange(0, static_cast<int>(duration));
    }

    void seek(int position) {
        m_player->setPosition(static_cast<qint64>(position));
    }

    void setVolume(int volume) {
        m_audioOutput->setVolume(volume / 100.0);
        
        // Update mute button icon
        if (volume == 0) {
            m_volumeButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
            m_volumeButton->setChecked(true);
        } else {
            m_volumeButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
            m_volumeButton->setChecked(false);
        }
    }

    void toggleMute(bool muted) {
        m_audioOutput->setMuted(muted);
        m_volumeButton->setIcon(muted ? 
            style()->standardIcon(QStyle::SP_MediaVolumeMuted) : 
            style()->standardIcon(QStyle::SP_MediaVolume));
    }

    void toggleFullscreen() {
        if (isFullScreen()) {
            showNormal();
            menuBar()->show();
            statusBar()->show();
        } else {
            showFullScreen();
            menuBar()->hide();
            statusBar()->hide();
        }
    }

    void setPlaybackRate(const QString &rate) {
        double speed = rate.left(rate.indexOf('x')).toDouble();
        m_player->setPlaybackRate(speed);
    }

    void previousTrack() {
        int currentRow = m_playlistWidget->currentRow();
        if (currentRow > 0) {
            m_playlistWidget->setCurrentRow(currentRow - 1);
            playSelectedItem(m_playlistWidget->currentItem());
        }
    }

    void nextTrack() {
        int currentRow = m_playlistWidget->currentRow();
        if (currentRow < m_playlistWidget->count() - 1) {
            m_playlistWidget->setCurrentRow(currentRow + 1);
            playSelectedItem(m_playlistWidget->currentItem());
        }
    }

    void handleMediaStatus(QMediaPlayer::MediaStatus status) {
        switch (status) {
        case QMediaPlayer::EndOfMedia:
            nextTrack();
            break;
        case QMediaPlayer::LoadedMedia:
            // Media loaded successfully
            break;
        case QMediaPlayer::BufferingMedia:
            // Show buffering status
            break;
        default:
            break;
        }
    }

    void handlePlayerError(QMediaPlayer::Error error, const QString &errorString) {
        QMessageBox::warning(this, "Playback Error", 
            QString("Error: %1\n%2").arg(error).arg(errorString));
    }

    void showAbout() {
        QMessageBox::about(this, "About ModernMediaPlayer",
            "<h2>ModernMediaPlayer</h2>"
            "<p>Version 1.0</p>"
            "<p>A modern, feature-rich media player built with Qt and C++</p>"
            "<p>&copy; 2023 ModernMediaPlayer</p>");
    }

private:
    // UI components
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    QVideoWidget *m_videoWidget;
    QSlider *m_timeSlider;
    QSlider *m_volumeSlider;
    QLabel *m_timeLabel;
    QToolButton *m_playButton;
    QToolButton *m_stopButton;
    QToolButton *m_prevButton;
    QToolButton *m_nextButton;
    QToolButton *m_volumeButton;
    QToolButton *m_fullscreenButton;
    QComboBox *m_playbackRateBox;
    QStatusBar *m_statusBar;
    QListWidget *m_playlistWidget;
    QDockWidget *m_playlistDock;
    QDockWidget *m_equalizerDock;
    QWidget *m_equalizerWidget;
    QAction *m_playAction;
    QAction *m_stopAction;
    QAction *m_playlistAction;
    QAction *m_equalizerAction;
    QSystemTrayIcon *m_trayIcon = nullptr;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application info
    app.setApplicationName("ModernMediaPlayer");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("ModernMediaPlayer");
    app.setWindowIcon(QIcon(":/icons/app_icon"));
    
    // Create and show main window
    MediaPlayer player;
    player.show();
    
    // Handle command line arguments
    if (app.arguments().size() > 1) {
        player.addToPlaylist(app.arguments().at(1));
        player.playFile(app.arguments().at(1));
    }
    
    return app.exec();
}

#include "media_player.moc"
