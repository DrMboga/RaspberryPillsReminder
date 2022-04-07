using System.Globalization;
using web_ui.model;

namespace web_ui.helpers
{
    public static class CsvHelper
    {
        public static List<T> ParseCsvToCollection<T>(this string csvText) where T : class, new()
        {
            var result = new List<T>();
            var tType = typeof(T);
            var props = tType.GetProperties();

            var csvLines = csvText.Split(Environment.NewLine);

            foreach (var csvLine in csvLines)
            {
                if (string.IsNullOrEmpty(csvLine))
                {
                    continue;
                }

                var csvFields = csvLine.Split("\t");
                if (csvFields.Length != props.Length)
                {
                    continue;
                }
                var instance = new T();
                for (int i = 0; i < csvFields.Length; i++)
                {
                    int intValue;
                    int boolAsIntValue;
                    DateTime dateTimeValue;
                    int ledActionAsInt;
                    if (props[i].PropertyType == typeof(int) && int.TryParse(csvFields[i], out intValue))
                    {
                        props[i].SetValue(instance, intValue);
                    }
                    if (props[i].PropertyType == typeof(bool) && int.TryParse(csvFields[i], out boolAsIntValue))
                    {
                        props[i].SetValue(instance, boolAsIntValue != 0);
                    }
                    if (props[i].PropertyType == typeof(DateTime) && DateTime.TryParseExact(csvFields[i].Trim(), "dd.MM.yyyy HH:mm.ss", null, DateTimeStyles.None, out dateTimeValue))
                    {
                        props[i].SetValue(instance, dateTimeValue);
                    }
                    if (props[i].PropertyType == typeof(string))
                    {
                        props[i].SetValue(instance, csvFields[i]);
                    }
                    if (props[i].PropertyType == typeof(LedAction) && int.TryParse(csvFields[i], out ledActionAsInt))
                    {
                        props[i].SetValue(instance, (LedAction)ledActionAsInt);
                    }
                }
                result.Add(instance);
            }
            return result;
        }
    }
}