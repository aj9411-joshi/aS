#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QMessageBox>
#include <QDateTime>
#include <QTimer>
#include <QRandomGenerator>
#include <QAbstractItemView>
#include <cmath>
#include <vector>
#include <QStringList>

extern "C" {
#include "collision.h"
#include "graph.h"
#include "routing.h"
}

struct SpaceObject {
    QString id;
    double x;
    double y;
    bool debris;
};

class OrbitWidget : public QWidget {
public:
    explicit OrbitWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setMinimumSize(430, 340);
        setStyleSheet("background:#071426;border:1px solid #1c3553;border-radius:12px;");
    }

    void setObjects(const std::vector<SpaceObject> *items, int warningA = -1, int warningB = -1) {
        objects = items;
        riskA = warningA;
        riskB = warningB;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), QColor("#071426"));

        const QPointF center(width()/2.0, height()/2.0);
        p.setPen(QPen(QColor("#173452"), 1, Qt::DashLine));
        for (int r : {65, 115, 165}) p.drawEllipse(center, r, r*0.62);
        p.setPen(QPen(QColor("#234565"), 1));
        p.drawLine(20, center.y(), width()-20, center.y());
        p.drawLine(center.x(), 20, center.x(), height()-20);

        p.setBrush(QColor("#173b61"));
        p.setPen(Qt::NoPen);
        p.drawEllipse(center, 19, 19);
        p.setPen(QColor("#8cc8ff"));
        p.drawText(QRectF(center.x()-35, center.y()+24, 70, 18), Qt::AlignCenter, "EARTH");

        if (!objects) return;
        for (int i=0; i<(int)objects->size(); ++i) {
            const auto &o = objects->at(i);
            double px = 45 + (o.x/300.0) * (width()-90);
            double py = 35 + (o.y/300.0) * (height()-70);
            QColor c = o.debris ? QColor("#ff7373") : QColor("#40c4ff");

            if (i == riskA || i == riskB) {
                p.setPen(QPen(QColor("#ff5252"), 2));
                p.setBrush(Qt::NoBrush);
                p.drawEllipse(QPointF(px,py), 13, 13);
            }
            p.setPen(Qt::NoPen);
            p.setBrush(c);
            p.drawEllipse(QPointF(px,py), 6, 6);
            p.setPen(QColor("#d9e9f8"));
            p.drawText(QPointF(px+9, py-7), o.id);
        }
    }

private:
    const std::vector<SpaceObject> *objects = nullptr;
    int riskA = -1, riskB = -1;
};

class MainWindow : public QMainWindow {
public:
    MainWindow() {
        setWindowTitle("ASTRA | Space Traffic & Risk Avoidance");
        resize(1180, 760);
        setStyleSheet(R"(
            QMainWindow, QWidget { background:#0b1728; color:#e8f1fb; font-family:'Segoe UI'; }
            QLabel#title { font-size:25px; font-weight:700; color:#ffffff; }
            QLabel#muted { color:#91a9c2; }
            QFrame#card { background:#102239; border:1px solid #1e3a58; border-radius:12px; }
            QPushButton { background:#1769aa; color:white; border:0; border-radius:7px; padding:10px 14px; font-weight:600; }
            QPushButton:hover { background:#2386d1; }
            QPushButton#secondary { background:#1a3049; border:1px solid #31516f; }
            QTableWidget { background:#0d1d30; alternate-background-color:#122840; gridline-color:#1d3854; border:0; }
            QHeaderView::section { background:#19334e; color:#cfe4f8; padding:7px; border:0; }
            QComboBox { background:#0d1d30; border:1px solid #31516f; padding:7px; border-radius:6px; }
        )");

        auto *central = new QWidget;
        auto *outer = new QVBoxLayout(central);
        outer->setContentsMargins(22,18,22,18);
        outer->setSpacing(14);

        auto *top = new QHBoxLayout;
        auto *heading = new QVBoxLayout;
        auto *title = new QLabel("ASTRA");
        title->setObjectName("title");
        auto *subtitle = new QLabel("Adaptive Space Traffic & Risk Avoidance System");
        subtitle->setObjectName("muted");
        heading->addWidget(title);
        heading->addWidget(subtitle);
        top->addLayout(heading);
        top->addStretch();
        statusLabel = new QLabel("● SYSTEM READY");
        statusLabel->setStyleSheet("color:#55d6a5;font-weight:700;padding:9px 13px;background:#12372f;border-radius:8px;");
        top->addWidget(statusLabel);
        outer->addLayout(top);

        auto *stats = new QHBoxLayout;
        objectsCount = makeStatCard("SPACE OBJECTS", "10");
        alertCount = makeStatCard("COLLISION ALERTS", "0");
        networkCount = makeStatCard("NETWORK LINKS", "0");
        stats->addWidget(objectsCount);
        stats->addWidget(alertCount);
        stats->addWidget(networkCount);
        outer->addLayout(stats);

        auto *middle = new QHBoxLayout;
        auto *orbitCard = new QFrame;
        orbitCard->setObjectName("card");
        auto *orbitLayout = new QVBoxLayout(orbitCard);
        auto *orbitTitle = new QLabel("ORBIT / OBJECT MONITOR");
        orbitTitle->setStyleSheet("font-weight:700;font-size:14px;");
        orbitLayout->addWidget(orbitTitle);
        orbit = new OrbitWidget;
        orbitLayout->addWidget(orbit,1);
        auto *legend = new QLabel("● Satellite     ● Space debris");
        legend->setStyleSheet("color:#9db6cf;padding:4px;");
        orbitLayout->addWidget(legend);

        auto *rightCard = new QFrame;
        rightCard->setObjectName("card");
        rightCard->setMinimumWidth(360);
        auto *rightLayout = new QVBoxLayout(rightCard);
        auto *tableTitle = new QLabel("OBJECT CATALOGUE");
        tableTitle->setStyleSheet("font-weight:700;font-size:14px;");
        rightLayout->addWidget(tableTitle);
        table = new QTableWidget(0,4);
        table->setHorizontalHeaderLabels({"Object ID","Type","X","Y"});
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->verticalHeader()->setVisible(false);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setAlternatingRowColors(true);
        rightLayout->addWidget(table,1);

        middle->addWidget(orbitCard, 3);
        middle->addWidget(rightCard, 2);
        outer->addLayout(middle, 1);

        auto *bottom = new QFrame;
        bottom->setObjectName("card");
        auto *bottomLayout = new QVBoxLayout(bottom);
        auto *controlsTitle = new QLabel("SIMULATION CONTROLS");
        controlsTitle->setStyleSheet("font-weight:700;font-size:14px;");
        bottomLayout->addWidget(controlsTitle);

        auto *buttons = new QHBoxLayout;
        auto *generateBtn = new QPushButton("Generate Demo Data");
        auto *collisionBtn = new QPushButton("Detect Collision");
        auto *networkBtn = new QPushButton("Build Space Network");
        auto *bfsBtn = new QPushButton("Run BFS");
        auto *dfsBtn = new QPushButton("Run DFS");
        auto *routeBtn = new QPushButton("Find Route");
        auto *safeRouteBtn = new QPushButton("Safest Route");
        auto *resetBtn = new QPushButton("Reset");
        resetBtn->setObjectName("secondary");
        for (auto *b : {generateBtn,collisionBtn,networkBtn,bfsBtn,dfsBtn,routeBtn,safeRouteBtn,resetBtn}) buttons->addWidget(b);
        bottomLayout->addLayout(buttons);

        auto *routeRow = new QHBoxLayout;
        routeRow->addWidget(new QLabel("Source:"));
        sourceBox = new QComboBox;
        routeRow->addWidget(sourceBox);
        routeRow->addWidget(new QLabel("Destination:"));
        destinationBox = new QComboBox;
        routeRow->addWidget(destinationBox);
        routeRow->addStretch();
        bottomLayout->addLayout(routeRow);

        resultLabel = new QLabel("Ready. Generate data, then detect collision or build the network.");
        resultLabel->setWordWrap(true);
        resultLabel->setStyleSheet("color:#b7cce0;background:#0b1b2e;padding:10px;border-radius:7px;");
        bottomLayout->addWidget(resultLabel);
        outer->addWidget(bottom);

        setCentralWidget(central);

        connect(generateBtn, &QPushButton::clicked, this, [this]{ generateData(); });
        connect(collisionBtn, &QPushButton::clicked, this, [this]{ detectCollisions(); });
        connect(networkBtn, &QPushButton::clicked, this, [this]{ buildNetwork(); });
        connect(bfsBtn, &QPushButton::clicked, this, [this]{ showTraversal(true); });
        connect(dfsBtn, &QPushButton::clicked, this, [this]{ showTraversal(false); });
        connect(routeBtn, &QPushButton::clicked, this, [this]{ findRoute(); });
        connect(safeRouteBtn, &QPushButton::clicked, this, [this]{ findSafestRoute(); });
        connect(resetBtn, &QPushButton::clicked, this, [this]{ generateData(); resultLabel->setText("Simulation reset. Demo objects have been restored."); });

        generateData();
    }

private:
    QFrame *makeStatCard(const QString &name, const QString &value) {
        auto *card = new QFrame;
        card->setObjectName("card");
        auto *layout = new QVBoxLayout(card);
        auto *n = new QLabel(name);
        n->setStyleSheet("color:#91a9c2;font-size:11px;font-weight:700;");
        auto *v = new QLabel(value);
        v->setStyleSheet("font-size:23px;font-weight:700;color:#ffffff;");
        layout->addWidget(n);
        layout->addWidget(v);
        if (name == "SPACE OBJECTS") objectValue = v;
        else if (name == "COLLISION ALERTS") alertValue = v;
        else linkValue = v;
        return card;
    }

    void generateData() {
        objects = {
            {"SAT-01",45,55,false},{"SAT-02",95,85,false},
            {"SAT-03",155,110,false},{"SAT-04",220,65,false},
            {"SAT-05",265,145,false},{"SAT-06",70,225,false},
            {"SAT-07",145,250,false},{"SAT-08",235,245,false},
            {"DEB-01",165,125,true},{"DEB-02",245,155,true}
        };
        highlightedA = highlightedB = -1;
        collisionAlerts = 0;
        networkReady = false;
        fillTable();
        orbit->setObjects(&objects);
        objectValue->setText(QString::number(objects.size()));
        alertValue->setText("0");
        linkValue->setText("0");
        sourceBox->clear();
        destinationBox->clear();
        for (int i=0;i<(int)objects.size();++i) {
            sourceBox->addItem(objects[i].id, i);
            destinationBox->addItem(objects[i].id, i);
        }
        destinationBox->setCurrentIndex(objects.size()-1);
        resultLabel->setText("10 demo objects loaded. Blue = satellite, red = debris. Positions are illustrative demo units.");
    }

    void fillTable() {
        table->setRowCount((int)objects.size());
        for (int i=0;i<(int)objects.size();++i) {
            const auto &o = objects[i];
            table->setItem(i,0,new QTableWidgetItem(o.id));
            table->setItem(i,1,new QTableWidgetItem(o.debris ? "Debris" : "Satellite"));
            table->setItem(i,2,new QTableWidgetItem(QString::number(o.x,'f',0)));
            table->setItem(i,3,new QTableWidgetItem(QString::number(o.y,'f',0)));
        }
    }

    void detectCollisions() {
        collisionAlerts = 0;
        double closest = 1e9;
        int a=-1,b=-1;
        QStringList lines;
        for (int i=0;i<(int)objects.size();++i) {
            for (int j=i+1;j<(int)objects.size();++j) {
                double d = calculateDistance(objects[i].x,objects[i].y,objects[j].x,objects[j].y);
                const char *risk = getRiskLevel(d);
                if (QString(risk) != "SAFE") {
                    ++collisionAlerts;
                    lines << QString("%1 ↔ %2 : %3 (distance %4)")
                        .arg(objects[i].id,objects[j].id,risk).arg(d,0,'f',1);
                }
                if (d < closest) { closest=d; a=i; b=j; }
            }
        }
        highlightedA=a; highlightedB=b;
        orbit->setObjects(&objects, highlightedA, highlightedB);
        alertValue->setText(QString::number(collisionAlerts));
        QString summary = collisionAlerts == 0 ? "No warning-level proximity found." :
            QString("%1 warning/critical object pairs found.\n%2").arg(collisionAlerts).arg(lines.join("\n"));
        resultLabel->setText(summary + QString("\nClosest pair: %1 and %2 (%3 units).")
                              .arg(objects[a].id,objects[b].id).arg(closest,0,'f',2));
    }

    void buildNetwork() {
        initializeGraph(&spaceGraph, (int)objects.size());
        initializeRouteGraph(&routeGraph, (int)objects.size());
        int links=0;
        const double linkRange=100.0;
        for (int i=0;i<(int)objects.size();++i) {
            for (int j=i+1;j<(int)objects.size();++j) {
                double d=calculateDistance(objects[i].x,objects[i].y,objects[j].x,objects[j].y);
                if (d <= linkRange) {
                    addUndirectedEdge(&spaceGraph,i,j,d);
                    addPathway(&routeGraph,i,j,d,edgeRisk(i,j),d*0.05);
                    ++links;
                }
            }
        }
        networkReady=true;
        linkValue->setText(QString::number(links));
        resultLabel->setText(QString("Space network built using a %1-unit connection range.\n%2 objects became nodes and %3 connections became edges.\nBFS/DFS and route finding can now be demonstrated.")
                             .arg(linkRange).arg(objects.size()).arg(links));
    }

    void showTraversal(bool useBfs) {
        if (!networkReady) buildNetwork();
        int order[MAX_NODES];
        int count = useBfs ? graphBFSOrder(&spaceGraph,0,order) : graphDFSOrder(&spaceGraph,0,order);
        QStringList values;
        for (int i=0;i<count;++i) values << objects[order[i]].id;
        resultLabel->setText(QString("%1 traversal starting from %2:\n%3")
                             .arg(useBfs ? "BFS (level by level)" : "DFS (go deep, then backtrack)")
                             .arg(objects[0].id, values.join("  →  ")));
    }

    /* Risk factor (0..1) of a link: 1.0 if it touches debris,
       0.2 if an endpoint passes within 40 units of debris, otherwise 0. */
    double edgeRisk(int a, int b) const {
        if (objects[a].debris || objects[b].debris) return 1.0;
        for (int n : {a, b}) {
            for (const auto &o : objects) {
                if (o.debris && calculateDistance(objects[n].x,objects[n].y,o.x,o.y) <= 40.0)
                    return 0.2;
            }
        }
        return 0.0;
    }

    void findSafestRoute() {
        if (!networkReady) buildNetwork();
        int src=sourceBox->currentData().toInt();
        int dst=destinationBox->currentData().toInt();
        RouteResult r = runDijkstra(&routeGraph, src, dst);
        if (!r.reachable) {
            resultLabel->setText("No connected route exists between the selected objects. Try another destination.");
            return;
        }
        QStringList route;
        for (int i=0;i<r.pathLength;++i) route << objects[r.path[i]].id;
        resultLabel->setText(QString("Safest route (Dijkstra, cost = distance x (1 + risk) + fuel):\n%1\nTotal weighted cost: %2")
                             .arg(route.join("  →  ")).arg(r.totalCost,0,'f',2));
    }

    void findRoute() {
        if (!networkReady) buildNetwork();
        int src=sourceBox->currentData().toInt();
        int dst=destinationBox->currentData().toInt();
        int path[MAX_NODES];
        int length=graphFindPath(&spaceGraph,src,dst,path);
        if (length==0) {
            resultLabel->setText("No connected route exists between the selected objects. Try another destination.");
            return;
        }
        QStringList route;
        for (int i=0;i<length;++i) route << objects[path[i]].id;
        double distance=graphPathWeight(&spaceGraph,path,length);
        resultLabel->setText(QString("Route found using graph search:\n%1\nTotal path distance: %2 demo units.")
                             .arg(route.join("  →  ")).arg(distance,0,'f',2));
    }

    std::vector<SpaceObject> objects;
    Graph spaceGraph{};
    RouteGraph routeGraph{};
    bool networkReady=false;
    int collisionAlerts=0;
    int highlightedA=-1, highlightedB=-1;
    OrbitWidget *orbit=nullptr;
    QTableWidget *table=nullptr;
    QComboBox *sourceBox=nullptr, *destinationBox=nullptr;
    QLabel *statusLabel=nullptr, *resultLabel=nullptr;
    QLabel *objectValue=nullptr, *alertValue=nullptr, *linkValue=nullptr;
    QFrame *objectsCount=nullptr, *alertCount=nullptr, *networkCount=nullptr;
};

int main(int argc, char *argv[]) {
    QApplication app(argc,argv);
    MainWindow window;
    window.show();
    return app.exec();
}
