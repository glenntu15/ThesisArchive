#include "threedview.h"
#include "surfacePlot.h"

#include <QtWidgets/QGridLayout>

ThreeDView::ThreeDView()
{

    mainLayout = nullptr;

    //Display();
}

ThreeDView::~ThreeDView()
{

}
void ThreeDView::Display(bool demo)
{
    if (mainLayout != nullptr){
        //clearLayout(mainLayout);
        delete mainLayout;
    }

    QWidget *widget = new QWidget;
    gridLayout = new QGridLayout(widget);
    //gridLayout->Name();

    Q3DSurface *graph = new Q3DSurface();
    container = QWidget::createWindowContainer(graph);
    psurface = new SurfacePlot(graph);

    if (demo)
        psurface->fillSqrtSinProxy();
    else {
        psurface->FillData();
    }

    QSize screenSize = graph->screen()->size();
    container->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 1.6));
    container->setMaximumSize(screenSize);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container->setFocusPolicy(Qt::StrongFocus);


    gridLayout->addWidget(container);

    widget->show();

    psurface->EnableModel(demo);
    //gridGroupBox = new QGroupBox(tr("grid group box"));
    //gridGroupBox->setLayout(gridLayout);

    //mainLayout->addWidget(gridGroupBox);

    //setLayout(mainLayout);
}

