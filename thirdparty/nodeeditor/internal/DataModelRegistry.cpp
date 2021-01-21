#include "DataModelRegistry.hpp"

#include <QtCore/QFile>
#include <QtWidgets/QMessageBox>

using QtNodes::DataModelRegistry;
using QtNodes::NodeDataModel;
using QtNodes::NodeDataType;
using QtNodes::TypeConverter;

std::unique_ptr<NodeDataModel> DataModelRegistry::create(QString const &modelName)
{
    auto it = m_registeredItemCreators.find(modelName);
    if (it != m_registeredItemCreators.end())
    {
        return it->second();
    }

    return nullptr;
}

DataModelRegistry::RegisteredModelCreatorsMap const& DataModelRegistry::registeredModelCreators() const
{
    return m_registeredItemCreators;
}

DataModelRegistry::RegisteredModelsCategoryMap const& DataModelRegistry::registeredModelsCategoryAssociation() const
{
    return m_registeredModelsCategory;
}

DataModelRegistry::CategoriesSet const& DataModelRegistry::categories() const
{
    return m_categories;
}

TypeConverter DataModelRegistry::getTypeConverter(NodeDataType const & d1,NodeDataType const & d2) const
{
    TypeConverterId converterId = std::make_pair(d1, d2);

    auto it = m_registeredTypeConverters.find(converterId);
    if (it != m_registeredTypeConverters.end())
    {
        return it->second;
    }

    return TypeConverter{};
}
