import Quickshell
import Quickshell.Triad

ShellRoot {
	Component.onCompleted: Triad.refresh()

	Variants {
		model: Triad.workspaces

		Scope {
			required property var modelData

			Component.onCompleted: {
				console.log(`${modelData.workspaceIndex}: ${modelData.name || modelData.tagId} ${modelData.layout}`)
			}
		}
	}
}
