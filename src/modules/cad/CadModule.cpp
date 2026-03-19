#include "CadModule.h"
#include "core/EventBus.h"
#include "core/DataModel.h"

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#include <QAction>
#include <QDebug>
#include <QElapsedTimer>

CadModule::CadModule(QObject* parent)
    : QObject(parent)
{
}

bool CadModule::initialize()
{
    // Listen for import requests
    connect(EventBus::instance(), &EventBus::eventPublished, this,
        [this](const QString& event, const QVariantMap& data) {
            if (event == "cad.import.request") {
                importStepFile(data.value("path").toString());
            }
        });

    return true;
}

void CadModule::shutdown()
{
    qDeleteAll(m_analyzers);
    m_analyzers.clear();
    m_faces.clear();
}

QList<QAction*> CadModule::menuActions()
{
    return {};
}

TopoDS_Face CadModule::face(int faceIndex) const
{
    if (faceIndex >= 0 && faceIndex < m_faces.size()) {
        return m_faces[faceIndex];
    }
    return TopoDS_Face();
}

SurfaceAnalyzer* CadModule::analyzerForFace(int faceIndex)
{
    if (m_analyzers.contains(faceIndex)) {
        return m_analyzers[faceIndex];
    }

    if (faceIndex >= 0 && faceIndex < m_faces.size()) {
        auto* analyzer = new SurfaceAnalyzer();
        analyzer->setFace(m_faces[faceIndex]);
        m_analyzers.insert(faceIndex, analyzer);
        return analyzer;
    }
    return nullptr;
}

void CadModule::importStepFile(const QString& path)
{
    auto* bus = EventBus::instance();
    auto* data = DataModel::instance();

    bus->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("Importing STEP file: %1").arg(path)}
    });

    QElapsedTimer timer;
    timer.start();

    // 1. Read STEP file
    if (!m_reader.load(path)) {
        bus->publish("log.message", {
            {"level", "ERROR"},
            {"message", m_reader.errorString()}
        });
        return;
    }

    bus->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("STEP file read complete, elapsed %1 ms").arg(timer.elapsed())}
    });

    // 2. Extract all faces
    m_faces.clear();
    qDeleteAll(m_analyzers);
    m_analyzers.clear();

    for (TopExp_Explorer explorer(m_reader.shape(), TopAbs_FACE);
         explorer.More(); explorer.Next()) {
        m_faces.append(TopoDS::Face(explorer.Current()));
    }

    bus->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("Extracted %1 faces").arg(m_faces.size())}
    });

    // 3. Analyze surface properties
    QVector<SurfaceInfo> surfaces = m_reader.analyzeSurfaces();

    bus->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("Surface analysis complete: %1 faces, total elapsed %2 ms")
                        .arg(surfaces.size())
                        .arg(timer.elapsed())}
    });

    // 4. Update data model
    data->setModelPath(path);
    data->setSurfaces(surfaces);

    // 5. Notify the Viewer module to display the model (pass shape pointer to avoid a round-trip event)
    bus->publish("cad.model.loaded", {
        {"path",      path},
        {"faceCount", m_faces.size()},
        {"shape_ptr", QVariant::fromValue(
            reinterpret_cast<quintptr>(&m_reader.shape()))}
    });

    bus->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("STEP model loaded successfully: %1").arg(path)}
    });
}
