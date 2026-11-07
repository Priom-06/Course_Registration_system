#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_loginBtn_clicked();
    void on_logoutBtn1_clicked();

    void on_studentConfirmBtn_clicked();
    void on_studentDropBtn_clicked();

    void on_adviserLoadBtn_clicked();
    void on_adviserAcceptBtn_clicked();

    void on_adminAddBtn_clicked();
    void on_adminEditBtn_clicked();
    void on_adminDeleteBtn_clicked();
    void on_adminSearchEdit_textChanged(const QString &text);

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;

    void initDatabase();
    void loadAvailableCourses();
    void loadAdviserRequests(const QString &studentId = "");
    void loadAdminCourses(const QString &filter = "");
};
#endif