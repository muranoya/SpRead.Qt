#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QDockWidget>
#include <QListWidget>
#include <QAction>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QFileInfo>
#include <QVector>
#include <QTimer>
#include <QCache>
#include <QByteArray>
#include <QThread>
#include <QMutex>

class ImageViewer : public QGraphicsView
{
    Q_OBJECT
public:
    enum ViewMode
    {
        FULLSIZE,
        FIT_WINDOW,
        FIT_IMAGE,
        CUSTOM_SCALE,
    };
    enum InterpolationMode
    {
        NearestNeighbor,
        Bilinear,
        Bicubic,
    };

    explicit ImageViewer(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~ImageViewer();

    void openImages(const QStringList &path);
    void clearPlaylist();

    QVector<int> histgram() const;

    void startSlideshow();
    void stopSlideshow();
    bool isPlayingSlideshow() const;

    void setSlideshowInterval(int msec);
    int getSlideshowInterval() const;

    void setSpreadMode(bool m);
    bool isSpreadMode() const;

    void setRightbindingMode(bool m);
    bool isRightbindingMode() const;

    void setScale(ViewMode m, qreal s);
    void setScale(ViewMode m);
    qreal getScale() const;
    ViewMode getScaleMode() const;

    void setInterpolationMode(InterpolationMode mode);
    InterpolationMode getInterpolationMode() const;

    void setOpenDirLevel(int n);
    int getOpenDirLevel() const;

    void setImageCacheSize(int cost);
    int getImageCacheSize() const;

    QSize orgImageSize(int i) const;
    QSize combinedImageSize() const;

    int countShowImages() const;
    int count() const;
    bool empty() const;

    int currentIndex(int i) const;
    QStringList currentFileNames() const;
    QString currentFileName(int i) const;

    QDockWidget *playlistDock() const;
    QString readableExtFormat() const;

signals:
    void stoppedSlideshow();
    void changeImage();

private slots:
    void menu_open_triggered();
    void menu_remove_triggered();
    void menu_clear_triggered();

    void playlistItemDoubleClicked(QListWidgetItem *item);
    void slideshow_loop();
    void prefetcher_prefetchFinished();
    void drag_check();

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

private:
    class File
    {
        public:
            enum FileType
            {
                INVALID,
                RAW,
                ARCHIVE,
            };

            explicit File(const QString &p, const QByteArray &rfilepath);
            explicit File(const QString &p);
            explicit File();
            virtual ~File();

            FileType fileType() const;
            QString physicalFilePath() const;
            QString physicalFileName() const;
            QString logicalFilePath() const;
            QString logicalFileName() const;

            QByteArray rawFilePath() const;

            QString createKey() const;

        private:
            FileType ft;
            QString archive_path;
            QString file_path;
            QByteArray raw_file_path;
    };

    class PlayListItem : public QListWidgetItem
    {
        public:
            explicit PlayListItem(const QString &p, const QByteArray &f,
                    QListWidget *parent = 0);
            explicit PlayListItem(const QString &f, QListWidget *parent = 0);
            virtual ~PlayListItem();
            File file() const;
        private:
            File f;
    };

    class Prefetcher : public QThread
    {
        public:
            Prefetcher(QCache<QString, QByteArray> *ch, QMutex *m);
            void setPrefetchImage(const QList<File> &list);
            void sendTermSig();

        protected:
            virtual void run();

        private:
            QCache<QString, QByteArray> *cache;
            QMutex *mutex;
            QList<File> plist;
            bool termsig;
    };

    // viewer
    QGraphicsScene *view_scene;
    QGraphicsPixmapItem *view_item;
    QVector<QImage> img_orgs; // 個々の画像
    QImage img_combined;      // 全ての画像を1枚の画像に連結したもの
    QImage img_scaled;        // 拡縮後の画像
    qreal scale_value;        // 表示倍率
    int img_count;            // 同時に表示している画像数
    ViewMode vmode;           // 表示モード
    InterpolationMode imode;  // 画素補完モード
    bool rightbinding;        // 見開き表示時に右綴じで表示するか
    QTimer drag_timer;
    bool is_drag_image;
    QPoint click_pos;
    QPoint move_pos;

    bool isReadableImageFile(const QString &path) const;
    void releaseImages();
    void showImages();
    void refresh();
    void setGraphicsPixmapItem(const QImage &img);
    void imageScale();
    void imageCombine(const QVector<QImage> &imgs);

    // playlist
    QDockWidget *playlistdock;
    QListWidget *playlist;
    QAction *menu_open;
    QAction *menu_sep1;
    QAction *menu_remove;
    QAction *menu_clear;
    QBrush normalBC;
    QBrush selectedBC;
    int index;
    bool spread_view;
    QTimer slideshow_timer;
    int slideshow_interval;
    int opendirlevel;

    bool validIndex(int i) const;
    void createPlaylistMenus();
    void playlistItemRemove(QList<QListWidgetItem*> items);
    void setHighlight();
    void clearHighlight();
    File currentFile(int i) const;
    QList<File> currentFiles() const;
    void nextImages();
    void previousImages();
    void openArchiveFile(const QString &path);
    void openFilesAndDirs(const QStringList &paths, int level);
    static QByteArray *readImageData(const File &f);
    static QByteArray *readArchiveData(const File &f);
    QByteArray *readData(const File &f);

    // prefetch
    QCache<QString, QByteArray> cache;
    Prefetcher prefetcher;
    QMutex prefetch_mutex;
    QList<QListWidgetItem*> prefetch_old;
    QList<QListWidgetItem*> prefetch_now;
    const QIcon prefetched_icon;

    void startPrefetch();

    // drag-and-drop
    bool isCopyDrop(const Qt::KeyboardModifiers km) const;
};

#endif // IMAGEMANAGER_H
