﻿using System;
using System.Windows;
using System.Windows.Navigation;
using System.Diagnostics;
using System.Collections.Generic;
using System.Linq;

namespace LauncherUI
{
	public partial class MainWindow : Window
	{
		class SaveRestoreData
		{
			public List<AddGameWindow.GameData> Games;
			public int SelectedIndex = 0;
			public string LaunchParameters;
		}

		void AddGamesToList()
		{
			if (!System.IO.File.Exists("LauncherUIData.json"))
			{
				return;
			}

			var saverestore = new SaveRestoreData();

			var json = System.IO.File.ReadAllText("LauncherUIData.json", new System.Text.UTF8Encoding(false));

			saverestore = Newtonsoft.Json.JsonConvert.DeserializeObject<SaveRestoreData>(json);

			LaunchOptionsTextBox.Text = saverestore.LaunchParameters;

			foreach (var item in saverestore.Games)
			{
				GameComboBox.Items.Add(item);
			}

			GameComboBox.SelectedIndex = saverestore.SelectedIndex;
		}

		public MainWindow()
		{
			InitializeComponent();
			AddGamesToList();

			ErrorText.Text = null;
		}

		void SaveGames()
		{
			var saverestore = new SaveRestoreData();
			saverestore.Games = GameComboBox.Items.Cast<AddGameWindow.GameData>().ToList();
			saverestore.LaunchParameters = LaunchOptionsTextBox.Text;
			saverestore.SelectedIndex = GameComboBox.SelectedIndex;

			var json = Newtonsoft.Json.JsonConvert.SerializeObject(saverestore);

			System.IO.File.WriteAllText("LauncherUIData.json", json, new System.Text.UTF8Encoding(false));
		}

		void Window_Closing(object sender, System.ComponentModel.CancelEventArgs args)
		{
			SaveGames();
		}

		void Hyperlink_RequestNavigate(object sender, RequestNavigateEventArgs args)
		{
			Process.Start(new ProcessStartInfo(args.Uri.AbsoluteUri));
			args.Handled = true;
		}

		void LaunchButton_Click(object sender, RoutedEventArgs args)
		{
			var options = LaunchOptionsTextBox.Text.Trim();
			var game = (AddGameWindow.GameData)GameComboBox.SelectedItem;
			var gamepath = game.GamePath;
			var exepath = game.ExecutablePath;

			if (!System.IO.Directory.Exists(gamepath))
			{
				ErrorText.Text = "Game path does not exist anymore.";
				return;
			}

			if (!System.IO.File.Exists(exepath))
			{
				ErrorText.Text = "Game executable path does not exist anymore.";
				return;
			}

			if (!System.IO.File.Exists("SourceDemoRender.dll"))
			{
				ErrorText.Text = "SourceDemoRender.dll does not exist.";
				return;
			}

			var launcher = "LauncherCLI.exe";

			if (!System.IO.File.Exists(launcher))
			{
				ErrorText.Text = "LauncherCLI.exe does not exist.";
				return;
			}

			var startparams = string.Format("/GAME \"{0}\" /PATH \"{1}\" /PARAMS \"{2}\"", exepath, gamepath, options);

			var info = new ProcessStartInfo(launcher, startparams);

			Process.Start(info);

			ErrorText.Text = null;
		}

		void AddGameButton_Click(object sender, RoutedEventArgs args)
		{
			var dialog = new AddGameWindow();
			dialog.Owner = this;

			dialog.ExistingGames = GameComboBox.Items.Cast<AddGameWindow.GameData>().ToList();
			dialog.OnGameAdded += OnGameAdded;

			dialog.ShowDialog();

			ErrorText.Text = null;
		}

		void RemoveGameButton_Click(object sender, RoutedEventArgs args)
		{
			ErrorText.Text = null;

			if (GameComboBox.SelectedItem == null)
			{
				return;
			}

			var index = GameComboBox.SelectedIndex;

			GameComboBox.Items.RemoveAt(index);
			GameComboBox.SelectedIndex = Math.Max(index - 1, 0);

			SaveGames();
		}

		void OnGameAdded(object sender, AddGameWindow.GameData args)
		{
			var index = GameComboBox.Items.Add(args);
			GameComboBox.SelectedIndex = index;

			SaveGames();
		}

		void GameComboBox_SelectionChanged(object sender, System.Windows.Controls.SelectionChangedEventArgs args)
		{
			ErrorText.Text = null;

			if (GameComboBox.Items.IsEmpty)
			{
				GameComboBox.ToolTip = null;
				return;
			}

			var obj = (AddGameWindow.GameData)GameComboBox.SelectedItem;

			/*
				This event gets called twice on removal, only use the second time.
			*/
			if (obj != null)
			{
				GameComboBox.ToolTip = string.Format("{0}\n\nExecutable\n{1}\n\nGame\n{2}", obj.DisplayName, obj.ExecutablePath, obj.GamePath);
			}
		}

		void ExtensionsButton_Click(object sender, RoutedEventArgs args)
		{
			var dialog = new ExtensionsWindow();
			dialog.Owner = this;

			dialog.ShowDialog();

			ErrorText.Text = null;
		}
	}
}
