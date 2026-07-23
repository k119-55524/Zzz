using System;
using System.Collections.Generic;

namespace editor.Services.Project.Infrastructure.Factories
{
    public static class AssetFactoryRegistry
    {
        private static readonly Dictionary<string, IAssetFactory> _factories = new(StringComparer.OrdinalIgnoreCase);
        private static readonly IAssetFactory _defaultFactory = new DefaultAssetFactory();

        static AssetFactoryRegistry()
        {
            Register(new SceneAssetFactory());
            Register(new ViewAssetFactory());
        }

        public static void Register(IAssetFactory factory)
        {
            _factories[factory.Extension] = factory;
        }

        public static IAssetFactory GetFactory(string extension)
        {
            if (_factories.TryGetValue(extension, out var factory))
            {
                return factory;
            }
            return _defaultFactory;
        }
    }
}
