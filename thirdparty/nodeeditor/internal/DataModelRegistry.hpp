#pragma once

#include <set>
#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>
#include <QtCore/QString>
#include "node/NodeDataModel.hpp"
#include "base/TypeConverter.hpp"
#include "base/Export.hpp"
#include "base/QStringStdHash.hpp"
#include "base/memory.hpp"

namespace QtNodes
{
	inline bool operator<(QtNodes::NodeDataType const & d1, QtNodes::NodeDataType const & d2)
	{
		return d1.id < d2.id;
	}

    /// Class uses map for storing models (name, model)
    class NODE_EDITOR_PUBLIC DataModelRegistry
    {
    public:
        using RegistryItemPtr     = std::unique_ptr<NodeDataModel>;
        using RegistryItemCreator = std::function<RegistryItemPtr()>;
        using RegisteredModelCreatorsMap = std::unordered_map<QString, RegistryItemCreator>;
        using RegisteredModelsCategoryMap = std::unordered_map<QString, QString>;
        using CategoriesSet = std::set<QString>;

        using RegisteredTypeConvertersMap = std::map<TypeConverterId, TypeConverter>;

        DataModelRegistry()  = default;
        ~DataModelRegistry() = default;

        DataModelRegistry(DataModelRegistry const &) = delete;
        DataModelRegistry(DataModelRegistry &&)      = default;

        DataModelRegistry&operator=(DataModelRegistry const &) = delete;
        DataModelRegistry&operator=(DataModelRegistry &&)      = default;

    public:
        void registerModel(const QString& name, const QString& category, std::function<std::unique_ptr<NodeDataModel>()> creator)
        {
			if (m_registeredItemCreators.count(name) == 0)
			{
				m_registeredItemCreators[name] = std::move(creator);
				m_categories.insert(category);
				m_registeredModelsCategory[name] = category;
			}
        }

        void registerTypeConverter(TypeConverterId const & id, TypeConverter typeConverter)
        {
            m_registeredTypeConverters[id] = std::move(typeConverter);
        }

        std::unique_ptr<NodeDataModel>create(QString const &modelName);

        RegisteredModelCreatorsMap const &registeredModelCreators() const;

        RegisteredModelsCategoryMap const &registeredModelsCategoryAssociation() const;

        CategoriesSet const &categories() const;

        TypeConverter getTypeConverter(NodeDataType const & d1, NodeDataType const & d2) const;

    private:
        RegisteredModelsCategoryMap m_registeredModelsCategory;
        CategoriesSet               m_categories;
        RegisteredModelCreatorsMap  m_registeredItemCreators;
        RegisteredTypeConvertersMap m_registeredTypeConverters;

    private:
        // If the registered ModelType class has the static member method
        //
        //      static Qstring Name();
        //
        // use it. Otherwise use the non-static method:
        //
        //       virtual QString name() const;

        template <typename T, typename = void>
        struct HasStaticMethodName : std::false_type
        {};

        template <typename T>
        struct HasStaticMethodName<T,typename std::enable_if<std::is_same<decltype(T::Name()), QString>::value>::type>
          : std::true_type
        {};
    };
}
