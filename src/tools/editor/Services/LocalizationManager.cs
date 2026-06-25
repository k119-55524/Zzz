using System;
using System.Globalization;
using System.Linq;
using System.Windows;

namespace editor.Services
{
    public static class LocalizationManager
    {
        public const string RussianCulture = "ru-RU";
        public const string EnglishCulture = "en-US";

        public static string CurrentCulture { get; private set; } = EnglishCulture;

        public static void Initialize(string savedLanguage)
        {
            string targetCulture = savedLanguage;

            if (string.IsNullOrEmpty(targetCulture))
            {
                // Auto-detect system language (Russian or English default)
                string systemLang = CultureInfo.CurrentUICulture.Name;
                if (systemLang.StartsWith("ru", StringComparison.OrdinalIgnoreCase))
                {
                    targetCulture = RussianCulture;
                }
                else
                {
                    targetCulture = EnglishCulture;
                }
            }

            SetLanguage(targetCulture);
        }

        public static void SetLanguage(string cultureCode)
        {
            if (cultureCode != RussianCulture && cultureCode != EnglishCulture)
            {
                cultureCode = EnglishCulture;
            }

            CurrentCulture = cultureCode;

            // Load the new dictionary
            string uriStr = $"Resources/Loc.{cultureCode}.xaml";
            var newDict = new ResourceDictionary
            {
                Source = new Uri(uriStr, UriKind.RelativeOrAbsolute)
            };

            // Remove old localization dictionary from merged dictionaries
            var mergedDicts = Application.Current.Resources.MergedDictionaries;
            var oldDict = mergedDicts.FirstOrDefault(d => d.Source != null && 
                (d.Source.OriginalString.Contains("Loc.ru-RU.xaml") || d.Source.OriginalString.Contains("Loc.en-US.xaml")));

            if (oldDict != null)
            {
                int index = mergedDicts.IndexOf(oldDict);
                mergedDicts[index] = newDict;
            }
            else
            {
                mergedDicts.Add(newDict);
            }
        }
    }
}
