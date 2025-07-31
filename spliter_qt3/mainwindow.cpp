#include "mainwindow.h"
#include "toolbar.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent), textEdit(new CodeEditor(this)), syntaxChecker(parser)
{
	DirSingleton::GetInstance().SetDir("C:\\testfolder");

	ToolBar* toolBar = new ToolBar(this);
	addToolBar(Qt::TopToolBarArea, toolBar);

	setCentralWidget(textEdit);
	resize(800, 600);
	setWindowTitle("Document Editor");

	highlighter = new ASTHighlighter(textEdit->document());

	createActions();
	textEdit->setTabStopDistance(4 * textEdit->fontMetrics().horizontalAdvance(' '));

	// ----- 입력 딜레이용 타이머 -----
	parseTimer = new QTimer(this);
	parseTimer->setSingleShot(true);   // 한 번만 실행되도록
	
	connect(textEdit, &QPlainTextEdit::textChanged, this, [this]() {
		if (analyzing) return;  // Analyze 중이면 무시
		if (parseTimer->isActive()) {
			parseTimer->stop();
		}
		parseTimer->start(1000);
	});

	connect(parseTimer, &QTimer::timeout, this, [this]() {
		analyzing = true;
		Analyze();
		analyzing = false;
	});

	connect(toolBar->openAction, &QAction::triggered, this, [this]() {
		QString fileName = QFileDialog::getOpenFileName(this, "파일 열기");
		if (!fileName.isEmpty()) {
			loadFile(fileName);   // 직접 만든 파일 로드 함수 호출
		}
	});


}

MainWindow::~MainWindow(){
}

void MainWindow::createActions(){
	exitAct = new QAction("Exit", this);
	connect(exitAct, &QAction::triggered, this, &MainWindow::exitApp);
}

void MainWindow::exitApp() {
	close();
}

void MainWindow::parsingText() {
	QString text = textEdit->toPlainText();
	std::string str = text.toStdString();
	parser.parsing(str);
}

void MainWindow::Analyze() {
	//구문 분석 실행
	parsingText();
	ASTList = parser.Getnodes();
	std::vector<Comment> comments = parser.GetComment();
	// QPlainTextEdit에서 전체 텍스트 가져오기
	QString fullText = textEdit->toPlainText();

	// 줄 단위로 분할
	QStringList lines = fullText.split('\n');

	// QStringList → std::vector<QString> 변환
	std::vector<QString> sourceLines;
	sourceLines.reserve(lines.size());
	for (const auto& line : lines) {
		sourceLines.push_back(line);
	}

	// 하이라이터에 AST와 함께 전달
	highlighter->clearHighlighter();
	highlighter->setAST(ASTList, sourceLines);
	highlighter->setComment(comments, sourceLines);

	//의미 분석 실행
	syntaxChecker.SyntaxCheck();

	highlighter->setErrors(syntaxChecker.GetError(), sourceLines);
	
}

void MainWindow::loadFile(const QString& fileName){
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::warning(this, "파일 열기 오류",
			"파일을 열 수 없습니다:\n" + file.errorString());
		return;
	}

	QTextStream in(&file);
	in.setEncoding(QStringConverter::Utf8);   // UTF-8 강제 설정
	QString content = in.readAll();

	textEdit->setPlainText(content);
	file.close();

	// 최근 열었던 파일 이름 저장 (예: 타이틀바 갱신)
	QFileInfo info(fileName);
	currentFile = info.filePath();   // currentFile은 MainWindow 멤버로 추가
	DirSingleton::GetInstance().SetDir(info.absolutePath().toUtf8().constData()); // 디렉토리 갱신
	setWindowTitle(QString("%1 - Makefile 편집기").arg(QFileInfo(fileName).fileName()));
}
